#include "main.h"

#define BUF_LEN 65535             /* バッファのサイズ */
#define BUFFSIZE 65535

void check_msg_from_parent();
static gint fd_check_io();
gint fd_recv();
gint fd_gets();
char *read_line();
void read_response();
gint fd_write();
void write_to_server();
#ifdef USE_SSL
void write_to_SSLserver();
#endif
void error();
void PortReq();
int sftp_c();
int sftp_get_c();
int ftp_c();

int http_c_nonssl();
#ifdef USE_SSL
int http_c_ssl();
#endif

void unchunk();


#ifdef USE_SSL
gint ssl_gets();
gint ssl_read();
gint ssl_peek();
gint ssl_write();
#endif

gboolean progress_timeout();

void dl_camz_list();

glong get_file_size();
void write_dlsz();
void unlink_dlsz();
glong get_dlsz();

static void cancel_http();
static void thread_cancel_http();


#ifdef POP_DEBUG
gboolean debug_flg=TRUE;
#else
gboolean debug_flg=FALSE;
#endif

char *my_strcasestr(const char *str, const char *pattern) {
    size_t i;

    if (!*pattern)
        return (char*)str;

    for (; *str; str++) {
        if (toupper(*str) == toupper(*pattern)) {
            for (i = 1;; i++) {
                if (!pattern[i])
                    return (char*)str;
                if (toupper(str[i]) != toupper(pattern[i]))
                    break;
            }
        }
    }
    return NULL;
}

gchar *make_rand16(){
  int i;
  gchar retc[17] ,*ret;
  gchar ch;

  srand ( time(NULL) );
  for ( i = 0 ; i < 16 ; i++ ) {
    ch = rand () % 62;
    if (ch>=52){
      retc[i]='0'+ch-52;
    }
    else if (ch>=26){
      retc[i]='A'+ch-26;
    }
    else{
      retc[i]='a'+ch;
    }
  }
  retc[i]=0x00;

  ret=g_strdup(retc);

  return(ret);
}

//タイムアウト付きコネクト(非同期コネクト)
int Connect(int socket, struct sockaddr * name, socklen_t namelen, gint timeout_sec)
{
  struct timeval timeout;
  int result, flags;
  fd_set readFd, writeFd, errFd;
  int sockNum;

  timeout.tv_sec=timeout_sec;
  timeout.tv_usec=0;
  
  //接続前に一度非同期に変更
  flags = fcntl(socket, F_GETFL);
  if(-1 == flags)
    {
      return -1;
    }
  result = fcntl(socket, F_SETFL, flags | O_NONBLOCK);
  if(-1 == result)
    {
        return -1;
    }
  
  //接続
  result = connect(socket, name, namelen);
  if(result == -1)
    {
      if(EINPROGRESS == errno)
	{
	  //非同期接続成功だとここに入る。select()で完了を待つ。
	  errno = 0;
        }
      else
        {
	  //接続失敗 同期に戻す。
	  fcntl(socket, F_SETFL, flags );
	  return -1;
        }
    }
  
  //同期に戻す。
  result = fcntl(socket, F_SETFL, flags );
  if(-1 == result)
    {
      //error
      return -1;
    }
  
  //セレクトで待つ
  FD_ZERO(&readFd);
  FD_ZERO(&writeFd);
  FD_ZERO(&errFd);
  FD_SET(socket, &readFd);
  FD_SET(socket, &writeFd);
  FD_SET(socket, &errFd);
  sockNum = select(socket + 1, &readFd, &writeFd, &errFd, &timeout);
  if(0 == sockNum)
    {
      //timeout error
      return -1;
    }
  else if(FD_ISSET(socket, &readFd) || FD_ISSET(socket, &writeFd) )
    {
      //読み書きできる状態
    }
  else
    {
      //error
      return -1;
    }

    //ソケットエラー確認
    int optval = 0;
    socklen_t optlen = (socklen_t)sizeof(optval);
    errno = 0;
    result = getsockopt(socket, SOL_SOCKET, SO_ERROR, (void *)&optval, &optlen);
    if(result < 0)
    {
        //error
    }
    else if(0 != optval)
    {
        //error
    }

    return 0;
}


void check_msg_from_parent(){
}

static gint fd_check_io(gint fd, GIOCondition cond)
{
	struct timeval timeout;
	fd_set fds;
	guint io_timeout=60;

	timeout.tv_sec  = io_timeout;
	timeout.tv_usec = 0;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	if (cond == G_IO_IN) {
		select(fd + 1, &fds, NULL, NULL,
		       io_timeout > 0 ? &timeout : NULL);
	} else {
		select(fd + 1, NULL, &fds, NULL,
		       io_timeout > 0 ? &timeout : NULL);
	}

	if (FD_ISSET(fd, &fds)) {
		return 0;
	} else {
		g_warning("Socket IO timeout\n");
		return -1;
	}
}

gint fd_recv(gint fd, gchar *buf, gint len, gint flags)
{
  gint ret;
  
  if (fd_check_io(fd, G_IO_IN) < 0)
    return -1;

  ret = recv(fd, buf, len, flags);
  return ret;
}


gint fd_gets(gint fd, gchar *buf, gint len)
{
  gchar *newline, *bp = buf;
  gint n;
  
  if (--len < 1)
    return -1;
  do {
    if ((n = fd_recv(fd, bp, len, MSG_PEEK)) <= 0)
      return -1;
    if ((newline = memchr(bp, '\n', n)) != NULL)
      n = newline - bp + 1;
    if ((n = fd_recv(fd, bp, n, 0)) < 0)
      return -1;
    bp += n;
    len -= n;
  } while (!newline && len);
  
  *bp = '\0';
  return bp - buf;
}

/*--------------------------------------------------
 * ソケットから1行読み込む
 */
char *read_line(int socket, char *p){
    char *org_p = p;

    while (1){
        if ( read(socket, p, 1) == 0 ) break;
        if ( *p == '\n' ) break;
        p++;
    }
    *(++p) = '\0';
    return org_p;
}


/*--------------------------------------------------
 * レスポンスを取得する。^\d\d\d- ならもう1行取得
 */
void read_response(int socket, char *p){
  int ret;
    do { 
      //read_line(socket, p);
    ret=fd_gets(socket,p,BUF_LEN);
        if ( debug_flg ){
	  fprintf(stderr, "<-- %s", p);fflush(stderr);
        }
    } while ( isdigit(p[0]) &&
	      isdigit(p[1]) && 
	      isdigit(p[2]) &&
	      p[3]=='-' );

}

gint fd_write(gint fd, const gchar *buf, gint len)
{
  if (fd_check_io(fd, G_IO_OUT) < 0)
    return -1;
  
  return write(fd, buf, len);
}

/*--------------------------------------------------
 * 指定されたソケット socket に文字列 p を送信。
 * 文字列 p の終端は \0 で terminate されている
 * 必要がある
 */

void write_to_server(int socket, char *p){
    if ( debug_flg ){
        fprintf(stderr, "--> %s", p);fflush(stderr);
    }
    
    fd_write(socket, p, strlen(p));
}

#ifdef USE_SSL
void write_to_SSLserver(SSL *ssl, char *p){
  if ( debug_flg ){
    fprintf(stderr, "[SSL] <-- %s", p);fflush(stderr);
  }
  
  ssl_write(ssl, p, strlen(p));
}
#endif

void error( char *message ){
  fprintf(stderr, "%s\n", message);
    exit(1);
}

void PortReq(char *IPaddr , int *i1 , int *i2 , int *i3 , int *i4 , int *i5 , int *i6)
{
  int j ;
  char *ip ;
  IPaddr = IPaddr + 3 ;

  while( isdigit(*IPaddr) == 0 ) { IPaddr++ ; }

  ip = strtok(IPaddr,",");
  *i1 = atoi(ip) ;

  ip = strtok(NULL,",");
  *i2 = atoi(ip) ;

  ip = strtok(NULL,",");
  *i3 = atoi(ip) ;

  ip = strtok(NULL,",");
  *i4 = atoi(ip) ;

  ip = strtok(NULL,",");
  *i5 = atoi(ip) ;

  ip = strtok(NULL,",");

  j = 0 ;
  while ( isdigit(*(ip +j)) != 0 ) { j += 1 ; }
  ip[j] = '\0' ;
  *i6 = atoi(ip) ;
}



gpointer thread_get_camz_list(gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;

  hl->pabort=FALSE;
  
  //http_c_nonssl(hl);
  http_c_ssl(hl);

  if(hl->ploop) g_main_loop_quit(hl->ploop);
}


void unchunk(gchar *dss_tmp){
  FILE *fp_read, *fp_write;
  gchar *unchunk_tmp;
  gchar cbuf[BUFFSIZE];
  gchar *dbuf=NULL;
  gchar *cpp;
  gchar *chunkptr, *endptr;
  long chunk_size;
  gint i, read_size=0, crlf_size=0;
  
  if ( debug_flg ){
    fprintf(stderr, "Decoding chunked file \"%s\".\n", dss_tmp);fflush(stderr);
  }
  
  fp_read=fopen(dss_tmp,"r");
  unchunk_tmp=g_strconcat(dss_tmp,"_unchunked",NULL);
  fp_write=fopen(unchunk_tmp,"wb");
  
  while(!feof(fp_read)){
    if(fgets(cbuf,BUFFSIZE-1,fp_read)){
      cpp=cbuf;
      
      read_size=strlen(cpp);
      for(i=read_size;i>=0;i--){
	if(isalnum(cpp[i])){
	  crlf_size=read_size-i-1;
	  break;
	}
	else{
	  cpp[i]='\0';
	}
      }
      chunkptr=g_strdup_printf("0x%s",cpp);
      chunk_size=strtol(chunkptr, &endptr, 0);
      g_free(chunkptr);
      
      if(chunk_size==0) break;
      
      if((dbuf = (gchar *)g_malloc(sizeof(gchar)*(chunk_size+crlf_size+1)))==NULL){
	fprintf(stderr, "!!! Memory allocation error in unchunk() \"%s\".\n", dss_tmp);
	fflush(stderr);
	break;
      }
      if(fread(dbuf,1, chunk_size+crlf_size, fp_read)){
	fwrite( dbuf , chunk_size , 1 , fp_write ); 
	if(dbuf) g_free(dbuf);
      }
      else{
	break;
      }
    }
  }
  
  fclose(fp_read);
  fclose(fp_write);
  
  unlink(dss_tmp);
  
  rename(unchunk_tmp,dss_tmp);
  
  g_free(unchunk_tmp);
}

#ifdef USE_SSL
 gint ssl_gets(SSL *ssl, gchar *buf, gint len)
{
  gchar *newline, *bp = buf;
  gint n;
  gint i;
  
  if (--len < 1)
    return -1;
  do {
    if ((n = ssl_peek(ssl, bp, len)) <= 0)
	return -1;
    if ((newline = memchr(bp, '\n', n)) != NULL)
      n = newline - bp + 1;
    if ((n = ssl_read(ssl, bp, n)) < 0)
      return -1;
    bp += n;
    len -= n;
  } while (!newline && len);
  
  *bp = '\0';
  return bp - buf;
}
#endif

#ifdef USE_SSL
 gint ssl_read(SSL *ssl, gchar *buf, gint len)
{
	gint err, ret;

	if (SSL_pending(ssl) == 0) {
		if (fd_check_io(SSL_get_rfd(ssl), G_IO_IN) < 0)
			return -1;
	}

	ret = SSL_read(ssl, buf, len);

	switch ((err = SSL_get_error(ssl, ret))) {
	case SSL_ERROR_NONE:
		return ret;
	case SSL_ERROR_WANT_READ:
	case SSL_ERROR_WANT_WRITE:
		errno = EAGAIN;
		return -1;
	case SSL_ERROR_ZERO_RETURN:
		return 0;
	default:
		g_warning("SSL_read() returned error %d, ret = %d\n", err, ret);
		if (ret == 0)
			return 0;
		return -1;
	}
}
#endif
 
/* peek at the socket data without actually reading it */
#ifdef USE_SSL
gint ssl_peek(SSL *ssl, gchar *buf, gint len)
{
	gint err, ret;

	if (SSL_pending(ssl) == 0) {
		if (fd_check_io(SSL_get_rfd(ssl), G_IO_IN) < 0)
			return -1;
	}

	ret = SSL_peek(ssl, buf, len);

	switch ((err = SSL_get_error(ssl, ret))) {
	case SSL_ERROR_NONE:
		return ret;
	case SSL_ERROR_WANT_READ:
	case SSL_ERROR_WANT_WRITE:
		errno = EAGAIN;
		return -1;
	case SSL_ERROR_ZERO_RETURN:
		return 0;
	case SSL_ERROR_SYSCALL:
	  // End of file
	  //printf("SSL_ERROR_SYSCALL ret=%d  %d\n",ret,(gint)strlen(buf));
	        return 0;
	default:
		g_warning("SSL_peek() returned error %d, ret = %d\n", err, ret);
		if (ret == 0)
			return 0;
		return -1;
	}
}
#endif

#ifdef USE_SSL
gint ssl_write(SSL *ssl, const gchar *buf, gint len)
{
	gint ret;

	ret = SSL_write(ssl, buf, len);

	switch (SSL_get_error(ssl, ret)) {
	case SSL_ERROR_NONE:
		return ret;
	case SSL_ERROR_WANT_READ:
	case SSL_ERROR_WANT_WRITE:
		errno = EAGAIN;
		return -1;
	default:
		return -1;
	}
}
#endif


int http_c_nonssl(typHLOG *hl)
{
  int command_socket;           /* コマンド用ソケット */
  int size;
  
  char send_mesg[BUF_LEN];          /* サーバに送るメッセージ */
  char buf[BUF_LEN+1];
  
  FILE *fp_write;
  FILE *fp_read;

  struct addrinfo hints, *res;
  struct in_addr addr;
  int err;

  gboolean chunked_flag=FALSE;
  gchar *cp;

  gchar *rand16=NULL;
  gint plen;

  check_msg_from_parent();
   
  /* ホストの情報 (IP アドレスなど) を取得 */
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;

  if ((err = getaddrinfo(hl->http_host, "http", &hints, &res)) !=0){
    fprintf(stderr, "Bad hostname [%s]\n", hl->http_host);
    return(HDSLOG_HTTP_ERROR_GETHOST);
  }

  check_msg_from_parent();
   
  /* ソケット生成 */
  if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
    fprintf(stderr, "Failed to create a new socket.\n");
    return(HDSLOG_HTTP_ERROR_SOCKET);
  }
  
  check_msg_from_parent();
   
  /* サーバに接続 */
  if( connect(command_socket, res->ai_addr, res->ai_addrlen) == -1){
    fprintf(stderr, "Failed to connect to %s .\n", hl->http_host);
    return(HDSLOG_HTTP_ERROR_CONNECT);
  }
  
  check_msg_from_parent();
   
  // AddrInfoの解放
  freeaddrinfo(res);

  // HTTP/1.1 ではchunked対策が必要
  sprintf(send_mesg, "GET %s HTTP/1.1\r\n", hl->http_path);
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "Accept: text/plain,text/html,application/x-gzip\r\n");
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "Accept-Encoding: gzip\r\n");
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "User-Agent: Mozilla/5.0\r\n");
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "Host: %s\r\n", hl->http_host);
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "Connection: close\r\n");
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "\r\n");
  write_to_server(command_socket, send_mesg);

  if((fp_write=fopen(hl->http_dlfile,"wb"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hl->http_dlfile);
    return(HDSLOG_HTTP_ERROR_TEMPFILE);
  }

  unlink_dlsz(hl);
  
  while((size = fd_gets(command_socket,buf,BUF_LEN)) > 2 ){
    // header lines
    if(debug_flg){
      fprintf(stderr,"--> Header: %s", buf);
    }
    if(NULL != (cp = strstr(buf, "Transfer-Encoding: chunked"))){
      chunked_flag=TRUE;
    }
    if(strncmp(buf,"Content-Length: ",strlen("Content-Length: "))==0){
      cp = buf + strlen("Content-Length: ");
      hl->http_dlsz=atol(cp);
    }
  }
  
  write_dlsz(hl);
  
  do{ // data read
    size = recv(command_socket,buf,BUF_LEN, 0);
    fwrite( &buf , size , 1 , fp_write ); 
  }while(size>0);
      
  fclose(fp_write);

  check_msg_from_parent();

  if(chunked_flag) unchunk(hl->http_dlfile);

    if((chmod(hl->http_dlfile,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
    g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hl->http_dlfile);
  }
  
  unlink_dlsz(hl);
  close(command_socket);

  return 0;
}

#ifdef USE_SSL
int http_c_ssl(typHLOG *hl)
{
  int command_socket;           /* コマンド用ソケット */
  int size;

  char send_mesg[BUF_LEN];          /* サーバに送るメッセージ */
  char buf[BUF_LEN+1];
  
  FILE *fp_write;
  FILE *fp_read;

  struct addrinfo hints, *res;
  struct addrinfo dhints, *dres;
  struct in_addr addr;
  int err, ret;

  gboolean chunked_flag=FALSE;
  gchar *cp;

  gchar *rand16=NULL;
  gint plen;

  SSL *ssl;
  SSL_CTX *ctx;

   
  check_msg_from_parent();

  /* ホストの情報 (IP アドレスなど) を取得 */
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;

  if ((err = getaddrinfo(hl->http_host, "https", &hints, &res)) !=0){
    fprintf(stderr, "Bad hostname [%s]\n", hl->http_host);
    return(HDSLOG_HTTP_ERROR_GETHOST);
  }

  check_msg_from_parent();

    /* ソケット生成 */
  if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
    fprintf(stderr, "Failed to create a new socket.\n");
    return(HDSLOG_HTTP_ERROR_SOCKET);
  }

  check_msg_from_parent();
  
  /* サーバに接続 */
  if( connect(command_socket, res->ai_addr, res->ai_addrlen) == -1){
    fprintf(stderr, "Failed to connect to %s .\n", hl->http_host);
    return(HDSLOG_HTTP_ERROR_CONNECT);
  }

  check_msg_from_parent();

  SSL_load_error_strings();
  SSL_library_init();

  ctx = SSL_CTX_new(SSLv23_client_method());
  ssl = SSL_new(ctx);
  err = SSL_set_fd(ssl, command_socket);
  while((ret=SSL_connect(ssl))!=1){
    err=SSL_get_error(ssl, ret);
    if( (err==SSL_ERROR_WANT_READ)||(err==SSL_ERROR_WANT_WRITE) ){
      g_usleep(100000);
      g_warning("SSL_connect(): try again\n");
      continue;
    }
    g_warning("SSL_connect() failed with error %d, ret=%d (%s)\n",
	      err, ret, ERR_error_string(ERR_get_error(), NULL));
    return (HDSLOG_HTTP_ERROR_CONNECT);
  }

  check_msg_from_parent();
  
  // AddrInfoの解放
  freeaddrinfo(res);

  // HTTP/1.1 ではchunked対策が必要
  sprintf(send_mesg, "GET %s HTTP/1.1\r\n", hl->http_path);
  write_to_SSLserver(ssl, send_mesg);

  sprintf(send_mesg, "Accept: application/xml, application/json\r\n");
  write_to_SSLserver(ssl, send_mesg);

  sprintf(send_mesg, "User-Agent: Mozilla/5.0\r\n");
  write_to_SSLserver(ssl, send_mesg);

  sprintf(send_mesg, "Host: %s\r\n", hl->http_host);
  write_to_SSLserver(ssl, send_mesg);

  sprintf(send_mesg, "Connection: close\r\n");
  write_to_SSLserver(ssl, send_mesg);

  sprintf(send_mesg, "\r\n");
  write_to_SSLserver(ssl, send_mesg);

  if((fp_write=fopen(hl->http_dlfile,"wb"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hl->http_dlfile);
    return(HDSLOG_HTTP_ERROR_TEMPFILE);
  }

  while((size = ssl_gets(ssl, buf, BUF_LEN)) > 2 ){
    // header lines
    if(debug_flg){
      fprintf(stderr,"[SSL] --> Header: %s", buf);
    }
    if(NULL != (cp = strstr(buf, "Transfer-Encoding: chunked"))){
      chunked_flag=TRUE;
      }
  }
  do{ // data read
    size = SSL_read(ssl, buf, BUF_LEN);
    fwrite( &buf , size , 1 , fp_write ); 
  }while(size >0);
      
  fclose(fp_write);

  check_msg_from_parent();

  if(chunked_flag) unchunk(hl->http_dlfile);

    if((chmod(hl->http_dlfile,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
    g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hl->http_dlfile);
  }

  SSL_shutdown(ssl);
  SSL_free(ssl);
  SSL_CTX_free(ctx);
  ERR_free_strings();
  
  close(command_socket);

  return 0;
}
#endif  //USE_SSL



gboolean progress_timeout( gpointer data ){
  typHLOG *hl=(typHLOG *)data;
  glong sz=-1;
  gchar *tmp;
  gdouble frac;

  if(!hl->http_ok){
    return(FALSE);
  }

  if(gtk_widget_get_realized(hl->pbar)){
    sz=get_file_size(hl->http_dlfile);

    if(sz>0){  // After Downloading Started to get current dlsz
      if(hl->http_dlsz<0){
	hl->http_dlsz=get_dlsz(hl);
      }
    }

    if(hl->http_dlsz>0){
      frac=(gdouble)sz/(gdouble)hl->http_dlsz;
      gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(hl->pbar),
				    frac);

      if(sz>1024*1024){
	tmp=g_strdup_printf("%d%% Downloaded (%.2lf / %.2lf MB)",
			    (gint)(frac*100.),
			    (gdouble)sz/1024./1024.,
			    (gdouble)hl->http_dlsz/1024./1024.);
      }
      else if(sz>1024){
	tmp=g_strdup_printf("%d%% Downloaded (%ld / %ld kB)",
			    (gint)(frac*100.),
			    sz/1024,
			    hl->http_dlsz/1024);
      }
      else{
	tmp=g_strdup_printf("%d%% Downloaded (%ld / %ld bytes)",
			    (gint)(frac*100.),
			    sz, hl->http_dlsz);
      }
    }
    else{
      gtk_progress_bar_pulse(GTK_PROGRESS_BAR(hl->pbar));

      if(sz>1024*1024){
	tmp=g_strdup_printf("Downloaded %.2lf MB",
			    (gdouble)sz/1024./1024.);
      }
      else if(sz>1024){
	tmp=g_strdup_printf("Downloaded %ld kB", sz/1024);
      }
      else if (sz>0){
	tmp=g_strdup_printf("Downloaded %ld bytes", sz);
      }
      else{
	tmp=g_strdup_printf("Waiting for HTTP server response ...");
      }
    }
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(hl->pbar),
			      tmp);
    g_free(tmp);
  }
  
  return TRUE;
}


glong get_file_size(gchar *fname)
{
  FILE *fp;
  long sz;

  fp = fopen( fname, "rb" );
  if( fp == NULL ){
    return -1;
  }

  fseek( fp, 0, SEEK_END );
  sz = ftell( fp );

  fclose( fp );
  return sz;
}


void write_dlsz(typHLOG *hl){
  FILE *fp;
  gchar *tmp_file;

  tmp_file=g_strdup_printf("%s%s%s-%d",
			   g_get_tmp_dir(), G_DIR_SEPARATOR_S,
			   HTTP_DLSZ_FILE, getuid());
  
  if((fp=fopen(tmp_file,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", tmp_file);
    g_free(tmp_file);
    return;
  }

  fprintf(fp, "%ld\n",hl->http_dlsz);
  fclose(fp);
  
  g_free(tmp_file);
  return;
}


void unlink_dlsz(typHLOG *hl){
  gchar *tmp_file;

  hl->http_dlsz=0;
  
  tmp_file=g_strdup_printf("%s%s%s-%d",
			   g_get_tmp_dir(), G_DIR_SEPARATOR_S,
			   HTTP_DLSZ_FILE, getuid());
  
  if(access(tmp_file, F_OK)==0){
    unlink(tmp_file);
  }

  g_free(tmp_file);
  return;
}


glong get_dlsz(typHLOG *hl){
  FILE *fp;
  gchar *tmp_file;
  glong sz=0;
  gchar buf[10];
  
  tmp_file=g_strdup_printf("%s%s%s-%d",
			   g_get_tmp_dir(), G_DIR_SEPARATOR_S,
			   HTTP_DLSZ_FILE, getuid());
  
  if((fp=fopen(tmp_file,"r"))==NULL){
    g_free(tmp_file);
    return(-1);
  }

  if(fgets(buf,10-1,fp)){
    sz = atol(buf);
  }
  fclose(fp);

  unlink(tmp_file);
  
  g_free(tmp_file);
  return (sz);
}



static void thread_cancel_http(GtkWidget *w, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;

  if(GTK_IS_WIDGET(hl->pdialog)) gtk_widget_unmap(hl->pdialog);

  g_cancellable_cancel(hl->pcancel);
  g_object_unref(hl->pcancel); 

  hl->pabort=TRUE;

  unlink_dlsz(hl);
  if(access(hl->http_dlfile, F_OK)==0) unlink(hl->http_dlfile);

  if(hl->ploop) g_main_loop_quit(hl->ploop);
}


int post_body_new(typHLOG *hl,
		  gboolean wflag,
		  int command_socket,
		  SSL *ssl, 
		  gchar *rand16,
		  gboolean SSL_flag){
  char send_mesg[BUF_LEN];          /* サーバに送るメッセージ */
  char ins_mesg[BUF_LEN];
  gint ip, plen, i, i_inst;
  gint i_obj, grism, exp, repeat, filter, gain, frames;
  gboolean sh, pc, ag, nw, queue;
  gchar *send_buf1=NULL, *send_buf2=NULL;
  gchar* sci_instrume=NULL, *tmp_inst=NULL;
  gboolean init_flag=FALSE;
  gboolean send_flag=TRUE;
  gchar *tmp;
  gint pa;
  gdouble dexp;

  {
    ip=0;
    plen=0;

    while(1){
      if(seimei_log_post[ip].key==NULL) break;
      send_flag=TRUE;

      switch(seimei_log_post[ip].flg){
      case POST_NULL:
	sprintf(send_mesg,
		"%s=&",
		seimei_log_post[ip].key);
	break;

      case POST_CONST:
	if(strcmp(seimei_log_post[ip].key,"Submit_remarks")==0){
	  sprintf(send_mesg,
		  "%s=%s",
		  seimei_log_post[ip].key,
		  seimei_log_post[ip].prm);
	}
	else{
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_log_post[ip].key,
		  seimei_log_post[ip].prm);
	}
	break;
	
      case POST_INPUT:
	if(strcmp(seimei_log_post[ip].key,"expid")==0){
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_log_post[ip].key,
		  hl->seimei_log_id);
	}
	else if(strcmp(seimei_log_post[ip].key,"remarks")==0){
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_log_post[ip].key,
		  (hl->seimei_log_txt) ? hl->seimei_log_txt : " ");
	}

	break;
      }


      if(send_flag){
	plen+=strlen(send_mesg);
	
	if(send_buf1) g_free(send_buf1);
	if(send_buf2) send_buf1=g_strconcat(send_buf2,send_mesg,NULL);
	else send_buf1=g_strdup(send_mesg);
	if(send_buf2) g_free(send_buf2);
	send_buf2=g_strdup(send_buf1);
      }

      ip++;
    }


    sprintf(send_mesg,"\r\n\r\n");
    if(send_buf1) g_free(send_buf1);
    send_buf1=g_strconcat(send_buf2,send_mesg,NULL);
    
    plen+=strlen(send_mesg);
    
    if(wflag){
      if(SSL_flag){
	write_to_SSLserver(ssl, send_buf1);
      }
      else{
	write_to_server(command_socket, send_buf1);
      }
    }

    if(send_buf1) g_free(send_buf1);
    if(send_buf2) g_free(send_buf2);

    // Seimei LOG
  }

  return(plen);
}


int http_c_fcdb_new(typHLOG *hl, gboolean SSL_flag, gboolean proxy_ssl){
  int command_socket;           /* コマンド用ソケット */
  int size;

  gboolean fcdb_post=TRUE;

  char send_mesg[BUF_LEN];          /* サーバに送るメッセージ */
  char buf[BUF_LEN+1];
  
  FILE *fp_write;
  FILE *fp_read;

  struct addrinfo hints, *res;
  struct sockaddr_in *addr_in;
  struct in_addr addr;
  int err, ret;

  gboolean chunked_flag=FALSE;
  gchar *cp;

  gchar *rand16=NULL;
  gint plen;

  SSL *ssl;
  SSL_CTX *ctx;

  // Calculate Content-Length
  if(fcdb_post){
    rand16=make_rand16();
    plen=post_body_new(hl, FALSE, 0, NULL, rand16, FALSE);
  }
   
  check_msg_from_parent();

  /* ホストの情報 (IP アドレスなど) を取得 */
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;

  if ((err = getaddrinfo((hl->proxy_flag) ? hl->proxy_host : hl->fcdb_host,
			 (SSL_flag) ? "https" : "http",
			 &hints, &res)) !=0){
    fprintf(stderr, "Bad hostname [%s]\n",
	    (hl->proxy_flag) ? hl->proxy_host : hl->fcdb_host);
    return(GRLOG_HTTP_ERROR_GETHOST);
  }

  check_msg_from_parent();

  if(hl->proxy_flag){
    addr_in = (struct sockaddr_in *)(res -> ai_addr);
    addr_in -> sin_port=htons(hl->proxy_port);
  }

  /* ソケット生成 */
  if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
    fprintf(stderr, "Failed to create a new socket.\n");
    return(GRLOG_HTTP_ERROR_SOCKET);
  }

  check_msg_from_parent();

  /* サーバに接続 */
  if( Connect(command_socket, res->ai_addr, res->ai_addrlen, 10) == -1){
    fprintf(stderr, "Failed to connect to %s .\n", hl->fcdb_host);
    return(GRLOG_HTTP_ERROR_CONNECT);
  }

  check_msg_from_parent();

  if(SSL_flag){  // HTTPS
    SSL_load_error_strings();
    SSL_library_init();
    
    ctx = SSL_CTX_new(SSLv23_client_method());
    ssl = SSL_new(ctx);
    err = SSL_set_fd(ssl, command_socket);
    while((ret=SSL_connect(ssl))!=1){
      err=SSL_get_error(ssl, ret);
      if( (err==SSL_ERROR_WANT_READ)||(err==SSL_ERROR_WANT_WRITE) ){
	g_usleep(100000);
	g_warning("SSL_connect(): try again\n");
	continue;
      }
      g_warning("SSL_connect() failed with error %d, ret=%d (%s)\n",
		err, ret, ERR_error_string(ERR_get_error(), NULL));
      return(GRLOG_HTTP_ERROR_SSL);
    }
    
    check_msg_from_parent();
  }
  
  // AddrInfoの解放
  freeaddrinfo(res);

  // HTTP/1.1 ではchunked対策が必要
  hl->psz=0;
  if(SSL_flag){  // HTTPS
    if(fcdb_post){
      sprintf(send_mesg, "POST %s HTTP/1.1\r\n", hl->fcdb_path);
    }
    else{
      sprintf(send_mesg, "GET %s HTTP/1.1\r\n", hl->fcdb_path);
    }
    write_to_SSLserver(ssl, send_mesg);
    
    
    sprintf(send_mesg, "User-Agent: Mozilla/5.0\r\n");
    write_to_SSLserver(ssl, send_mesg);
    
    sprintf(send_mesg, "Host: %s\r\n", hl->fcdb_host);
    write_to_SSLserver(ssl, send_mesg);
  }
  else{  // HTTP
    if(hl->proxy_flag){
      if(proxy_ssl){
	if(fcdb_post){
	  sprintf(send_mesg, "POST https://%s%s HTTP/1.1\r\n",
		  hl->fcdb_host,hl->fcdb_path);
	}
	else{
	  sprintf(send_mesg, "GET https://%s%s HTTP/1.1\r\n",
		  hl->fcdb_host,hl->fcdb_path);
	}
      }
      else{ 
	if(fcdb_post){
	  sprintf(send_mesg, "POST http://%s%s HTTP/1.1\r\n",
		  hl->fcdb_host,hl->fcdb_path);
	}
	else{
	  sprintf(send_mesg, "GET http://%s%s HTTP/1.1\r\n",
		  hl->fcdb_host,hl->fcdb_path);
	}
      }
    }
    else{
      if(fcdb_post){
	sprintf(send_mesg, "POST %s HTTP/1.1\r\n", hl->fcdb_path);
      }
      else{
	sprintf(send_mesg, "GET %s HTTP/1.1\r\n", hl->fcdb_path);
      }
    }
    write_to_server(command_socket, send_mesg);
    
    sprintf(send_mesg, "User-Agent: Mozilla/5.0\r\n");
    write_to_server(command_socket, send_mesg);

    sprintf(send_mesg, "Host: %s\r\n", hl->fcdb_host);
    write_to_server(command_socket, send_mesg);
  }

  // Header
  {
    sprintf(send_mesg, "Accept: application/xml, application/json\r\n");
    if(SSL_flag){  // HTTPS
      write_to_SSLserver(ssl, send_mesg);   
    }
    else{  // HTTP
      write_to_server(command_socket, send_mesg);
    }
  }
  
  sprintf(send_mesg, "Connection: close\r\n");
  if(SSL_flag){  // HTTPS
    write_to_SSLserver(ssl, send_mesg);
  }
  else{  // HTTP
    write_to_server(command_socket, send_mesg);
  }


  /////////// POST
  if(fcdb_post){
    sprintf(send_mesg, "Content-Length: %d\r\n", plen);
    if(SSL_flag){  // HTTPS
      write_to_SSLserver(ssl, send_mesg);
    }
    else{  // HTTP
      write_to_server(command_socket, send_mesg);
    }

    sprintf(send_mesg, "Content-Type: application/x-www-form-urlencoded\r\n");

    if(SSL_flag){  // HTTPS
      write_to_SSLserver(ssl, send_mesg);
    }
    else{  // HTTP
      write_to_server(command_socket, send_mesg);
    }
  }

  sprintf(send_mesg, "\r\n");
  if(SSL_flag){  // HTTPS
    write_to_SSLserver(ssl, send_mesg);
  }
  else{  // HTTP
    write_to_server(command_socket, send_mesg);
  }
  
  // POST body
  if(fcdb_post){
    if(SSL_flag){
      plen=post_body_new(hl, TRUE, 0, ssl, rand16, TRUE);
    }
    else{
      plen=post_body_new(hl, TRUE, command_socket, NULL, rand16, FALSE);
    }
    if(rand16) g_free(rand16);
  }

  // Download a file
  if((fp_write=fopen(hl->fcdb_file,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hl->fcdb_file);
    return(GRLOG_HTTP_ERROR_TEMPFILE);
  }

  
  if(SSL_flag){  // HTTPS
    while((size = ssl_gets(ssl, buf, BUF_LEN)) > 2 ){
      // header lines
      if(debug_flg){
	fprintf(stderr,"[SSL] --> Header: %s", buf);
      }
      if(NULL != (cp = my_strcasestr(buf, "Transfer-Encoding: chunked"))){
	chunked_flag=TRUE;
      }
      if(strncmp(buf,"Content-Length: ",strlen("Content-Length: "))==0){
	cp = buf + strlen("Content-Length: ");
	hl->psz=atol(cp);
      }
    }
    do{ // data read
      size = SSL_read(ssl, buf, BUF_LEN);
      fwrite( &buf , size , 1 , fp_write ); 
    }while(size >0);
  }
  else{  // HTTP
    while((size = fd_gets(command_socket,buf,BUF_LEN)) > 2 ){
      // header lines
      if(debug_flg){
	fprintf(stderr,"--> Header: %s", buf);
      }
      if(NULL != (cp = my_strcasestr(buf, "Transfer-Encoding: chunked"))){
	chunked_flag=TRUE;
      }
      if(strncmp(buf,"Content-Length: ",strlen("Content-Length: "))==0){
	cp = buf + strlen("Content-Length: ");
	hl->psz=atol(cp);
      }
    }
    do{ // data read
      size = recv(command_socket,buf,BUF_LEN, 0);
      fwrite( &buf , size , 1 , fp_write ); 
    }while(size>0);
  }
      
  fclose(fp_write);

  check_msg_from_parent();

  //if(chunked_flag) unchunk(hg->fcdb_file);

  if((chmod(hl->fcdb_file,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
    g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hl->fcdb_file);
  }

  if(SSL_flag){ // HTTPS
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    ERR_free_strings();
  }
  
  close(command_socket);

  return 0;
}

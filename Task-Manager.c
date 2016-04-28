/*
TO COMPILE :
	 gcc Task-Manager.c -o Task-Manager `pkg-config --cflags --libs gtk+-3.0 gthread-2.0` -w

*/

#include<gtk/gtk.h>
#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<pthread.h>

//GLOBAL DECLARATIONS.

//Variables for New-Connection.
	GtkBuilder *new_connection_ui;
	GObject *new_connection_window,*new_connection_okay,*new_connection_cancel;
//Variables for New-Connection ends.

//Variables for New-Connection-Loading.
	GObject *loading_connection_window,*loading_connection_cancel;
	GObject *loading_connection_spinner;
//Variables for New-Connection-Loading ends.

//Variables for Task-Manager.
	GtkBuilder *task_manager_ui;
	GObject *task_manager_window,*menubar,*menu_file,*menu_view,*menu_options,*menu_help;
	GObject *file_newcon,*file_run,*file_exit;
	GObject *options_ontop,*options_minclose,*options_color;
	GObject *view_refreshnow,*view_refreshspeed;
	GObject *help_about;
	GtkLabel *cpuusage,*memusage;
//Variable for Task-Manager ends.

//SOCKET DECLARATIONS

	int taskSock=-1,memSock=-1,cpuSock=-1,runSock=-1;
	char taskBuffer[128],memBuffer[128],cpuBuffer[128],runBuffer[128];
	int refreshRate=10;
	int refreshStop=0;

//SOCKET DECLARATIONS ENDS

//GLOBAL DECLARATIONS ENDS.


//SOCKETS START

void Die(char *mess) { perror(mess); return; }

int receiveDataAndCreateTasks(int sock,char *buffer){
	int received = 0;
	int bytes=0;
	char command[50],pid[10],cpu[5],mem[5],user[50];
	if ((bytes = recv(sock, buffer, 128, 0)) < 1) {
		Die("Failed to receive bytes from server");
		return 0;
	}

	while (strcmp(buffer,"DONE")!=0) {
		received += bytes;
		buffer[bytes] = '\0';        /* Assure null terminated string */
		sscanf(buffer,"%s%s%s%s%s",command,pid,cpu,mem,user);
		printf("Task : %s->%s->%s->%s->%s\n",command,pid,cpu,mem,user);
		bytes=0;
		if ((bytes = recv(sock, buffer, 128, 0)) < 1) {
			Die("Failed to receive bytes from server");
			return 0;
		}
	}
	return 1;
}

int receiveCPUUsage(int sock,char *buffer){
	int received = 0;
	int bytes=0;
	char usage[5];
	if ((bytes = recv(sock, buffer, 128, 0)) < 1) {
		Die("Failed to receive bytes from server");
		return 0;
	}

	while (strcmp(buffer,"DONE")!=0) {
		received += bytes;
		buffer[bytes] = '\0';        /* Assure null terminated string */
		sscanf(buffer,"%s",usage);
		printf("CPU Usage : %s\n",usage);
		bytes=0;
		if ((bytes = recv(sock, buffer, 128, 0)) < 1) {
			Die("Failed to receive bytes from server");
			return 0;
		}
	}
	cpuusage=GTK_WIDGET(gtk_builder_get_object(task_manager_ui,"cpu-usage-label"));
	gtk_label_set_text(cpuusage, usage);
	return 1;
}

int sendMessageOverSocket(int sock,char *msg){
	if (send(sock, msg, 128, 0) != 128) {
		Die("Mismatch in number of sent bytes");
		return 0;
	}
	
	return 1;
}

int receiveMemUsage(int sock,char *buffer){
	int received = 0;
	int bytes=0;
	char usage[5];
	if ((bytes = recv(sock, buffer, 128, 0)) < 1) {
		Die("Failed to receive bytes from server");
		return 0;
	}

	while (strcmp(buffer,"DONE")!=0) {
		received += bytes;
		buffer[bytes] = '\0';        /* Assure null terminated string */
		sscanf(buffer,"%s",usage);
		printf("Memory Usage : %s\n",usage);
		bytes=0;
		if ((bytes = recv(sock, buffer, 128, 0)) < 1) {
			Die("Failed to receive bytes from server");
			return 0;
		}
	}
	memusage=GTK_WIDGET(gtk_builder_get_object(task_manager_ui,"mem-usage-label"));
	gtk_label_set_text(memusage, usage);
	return 1;
}


int setupSocket(char *ip,int port){
	printf("Setting up socket.\n");
	struct sockaddr_in echoserver;
            
	/* Create the TCP socket */
	if ((taskSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0||(cpuSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0||(memSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0||(runSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		Die("Failed to create socket");
		taskSock=runSock=cpuSock=memSock=-1;
		return 0;
	}

	/* Construct the server sockaddr_in structure */
	memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
	echoserver.sin_family = AF_INET;                  /* Internet/IP */
	echoserver.sin_addr.s_addr = inet_addr(ip);  /* IP address */
	echoserver.sin_port = htons(port);       /* server port */
	/* Establish connection */
	if (connect(taskSock,(struct sockaddr *) &echoserver,sizeof(echoserver)) < 0||connect(cpuSock,(struct sockaddr *) &echoserver,sizeof(echoserver)) < 0||connect(memSock,(struct sockaddr *) &echoserver,sizeof(echoserver)) < 0||connect(runSock,(struct sockaddr *) &echoserver,sizeof(echoserver)) < 0) {
		Die("Failed to connect with server");
		taskSock=runSock=cpuSock=memSock=-1;
		return 0;
	}
	printf("Socket created succesfully.\n");
	return 1;

	//01 COMMANDS [ Get tasks ] : whoami | xargs top -b -n 1 -u | awk '{if(NR>7)printf "%-s %6s %-4s %-4s %-4s\n",$NF,$1,$9,$10,$2}'
	//02 COMMANDS [ Get CPU% Usage ] : top -bn1 | grep "Cpu(s)" | sed "s/.*, *\([0-9.]*\)%* id.*/\1/" | awk '{print $1"%"}'	
	//03 COMMANDS [ Get Mem% Usage ] : free -m | grep Mem | awk '{printf("%0.1f%s",$3/$2*100,"%")}'
}

//SOCKET ENDS

void shutDown(GtkWidget *widget,gpointer data){
	strcpy(taskBuffer,"BYE");	
	sendMessageOverSocket(taskSock,taskBuffer);
	sendMessageOverSocket(cpuSock,taskBuffer);
	sendMessageOverSocket(memSock,taskBuffer);
	sendMessageOverSocket(runSock,taskBuffer);
	close(taskSock);
	close(cpuSock);
	close(memSock);
	close(runSock);
	taskSock=runSock=cpuSock=memSock=-1;
	gtk_widget_destroy(task_manager_window);
	gtk_main_quit();
}

void closeNewConnectionWindow(GtkWidget *widget,GtkWidget *window){
	if(window!=NULL)
		gtk_widget_destroy(window);
	gtk_widget_set_sensitive(task_manager_window,TRUE);
}

static void printMenuActivatedData(GtkWidget *widget,gpointer data){
  g_print("\n%s was activated.\n",(gchar *)data);
}

int validateIP(char *ip){
	unsigned part1, part2, part3, part4;
	unsigned char c;

	if(sscanf(ip,"%3u.%3u.%3u.%3u%c",&part1,&part2,&part3,&part4,&c) != 4)
		return 0;

	if((part1|part2|part3|part4)>255)
		return 0;

	if(strspn(ip,"0123456789.")<strlen(ip))
		return 0;

	return 1;
}

void *loadWindowThreadFunction(){
	gtk_widget_set_sensitive(task_manager_window,FALSE);
	loading_connection_window = gtk_dialog_new();
	GtkWidget *content_area;
	GtkWidget *box=gtk_builder_get_object(new_connection_ui,"loading-box");
	
	loading_connection_cancel=gtk_builder_get_object(new_connection_ui,"loading-cancel-button");
	g_signal_connect(loading_connection_cancel,"clicked",G_CALLBACK(closeNewConnectionWindow),loading_connection_window);

	loading_connection_spinner=gtk_builder_get_object(new_connection_ui,"loading-spinner");
	gtk_spinner_start(loading_connection_spinner);

	content_area = gtk_dialog_get_content_area(GTK_DIALOG(loading_connection_window));
	//gtk_widget_reparent(box,GTK_CONTAINER(content_area));
	gtk_container_add(GTK_CONTAINER(content_area), box);

	g_signal_connect(loading_connection_window, "destroy", G_CALLBACK(closeNewConnectionWindow),loading_connection_window);
	gtk_window_set_title(loading_connection_window,"Loading Tasks");
	gtk_window_set_skip_taskbar_hint(loading_connection_window,TRUE);
	gtk_window_set_resizable(GTK_WINDOW(loading_connection_window), FALSE);

	gtk_widget_show_all(loading_connection_window);
}

struct conDetail{
	char ip[16];
	char port[8];
};

void threadedSendReceiveTasks(){
	if(taskSock==-1||cpuSock==-1||memSock==-1||runSock==-1||refreshStop)
		return;
	sendMessageOverSocket(taskSock,"whoami | xargs top -b -n 1 -u | awk \'{if(NR>7)printf \"%-s %6s %-4s %-4s %-4s\\n\",$NF,$1,$9,$10,$2}\'");
	receiveDataAndCreateTasks(taskSock,taskBuffer);
}

void threadedSendReceiveCPU(){
	if(taskSock==-1||cpuSock==-1||memSock==-1||runSock==-1||refreshStop)
		return;
	sendMessageOverSocket(cpuSock,"top -bn2 | grep \"Cpu(s)\" | sed \"s/.*, *\\([0-9.]*\\)%* id.*/\\1/\" | awk \'{print 100-$1\"%\"}\'");
	receiveCPUUsage(cpuSock,cpuBuffer);
}

void threadedSendReceiveMem(){
	if(taskSock==-1||cpuSock==-1||memSock==-1||runSock==-1||refreshStop)
		return;
	sendMessageOverSocket(memSock,"free -m | grep Mem | awk \'{printf(\"%0.1f%s\",$3/$2*100,\"%\")}\'");
	receiveMemUsage(memSock,memBuffer);
}

void *socketSetupThreadFunction(void *detail){
	//sleep(5)
	struct conDetail *det;
	det=(struct conDetial *) detail;
	if(taskSock!=-1||cpuSock!=-1||memSock!=-1||runSock!=-1){
		close(taskSock);
		close(cpuSock);
		close(memSock);
		close(runSock);
	}
	//gdk_threads_enter ();
	if(setupSocket(det->ip,atoi(det->port))==0){
		closeNewConnectionWindow(NULL,loading_connection_window);
		createNewConnectionWindow(NULL,NULL);
		GObject *ip_add=gtk_builder_get_object(new_connection_ui,"ip-entry");
		GObject *port_add=gtk_builder_get_object(new_connection_ui,"port-entry");
		GdkColor red;
		gdk_color_parse("red",&red);
		gtk_widget_modify_fg(GTK_WIDGET(ip_add),GTK_STATE_NORMAL,&red);
		gtk_widget_modify_fg(GTK_WIDGET(port_add),GTK_STATE_NORMAL,&red);
		gtk_entry_set_text(GTK_ENTRY(ip_add),det->ip);
		gtk_entry_set_text(GTK_ENTRY(port_add),det->port);
		return;
	}

	closeNewConnectionWindow(NULL,loading_connection_window);
	
	sendMessageOverSocket(taskSock,"whoami | xargs top -b -n 1 -u | awk \'{if(NR>7)printf \"%-s %6s %-4s %-4s %-4s\\n\",$NF,$1,$9,$10,$2}\'");
	receiveDataAndCreateTasks(taskSock,taskBuffer);
	sendMessageOverSocket(cpuSock,"top -bn2 | grep \"Cpu(s)\" | sed \"s/.*, *\\([0-9.]*\\)%* id.*/\\1/\" | awk \'{print 100-$1\"%\"}\'");
	receiveCPUUsage(cpuSock,cpuBuffer);
	sendMessageOverSocket(memSock,"free -m | grep Mem | awk \'{printf(\"%0.1f%s\",$3/$2*100,\"%\")}\'");
	receiveMemUsage(memSock,memBuffer);

	g_timeout_add_seconds(refreshRate,threadedSendReceiveTasks,NULL);
	g_timeout_add_seconds(refreshRate,threadedSendReceiveCPU,NULL);
	g_timeout_add_seconds(refreshRate,threadedSendReceiveMem,NULL);
	//gdk_threads_leave();
}

void startConnection(char *ip,char *port){

	gtk_widget_set_sensitive(task_manager_window,FALSE);
	loading_connection_window = gtk_dialog_new();
	GtkWidget *content_area;
	GtkWidget *box=gtk_builder_get_object(new_connection_ui,"loading-box");
	
	loading_connection_cancel=gtk_builder_get_object(new_connection_ui,"loading-cancel-button");
	g_signal_connect(loading_connection_cancel,"clicked",G_CALLBACK(closeNewConnectionWindow),loading_connection_window);

	loading_connection_spinner=gtk_builder_get_object(new_connection_ui,"loading-spinner");
	gtk_spinner_start(loading_connection_spinner);

	content_area = gtk_dialog_get_content_area(GTK_DIALOG(loading_connection_window));
	//gtk_widget_reparent(box,GTK_CONTAINER(content_area));
	gtk_container_add(GTK_CONTAINER(content_area), box);

	g_signal_connect(loading_connection_window, "destroy", G_CALLBACK(closeNewConnectionWindow),loading_connection_window);
	gtk_window_set_title(loading_connection_window,"Loading Tasks");
	gtk_window_set_skip_taskbar_hint(loading_connection_window,TRUE);
	gtk_window_set_resizable(GTK_WINDOW(loading_connection_window), FALSE);

	gtk_widget_show_all(loading_connection_window);	

	struct conDetail detail;
	strcpy(detail.ip,ip);
	strcpy(detail.port,port);

	GThread *startLoadingWindowThread,*startSetupSocketThread;
	GError *err1 = NULL ;
	GError *err2 = NULL ;

	/*if((startLoadingWindowThread=g_thread_create((GThreadFunc)loadWindowThreadFunction,NULL,TRUE,&err1))==NULL){
		printf("Thread create failed: %s\n",err1->message);
		g_error_free(err1);
	}*/

	if((startSetupSocketThread=g_thread_create((GThreadFunc)socketSetupThreadFunction,(void *)&detail,TRUE,&err2))==NULL){
		printf("Thread create failed: %s\n",err2->message);
		g_error_free(err2) ;
	}

	//g_thread_join(startLoadingWindowThread);
	g_thread_join(startSetupSocketThread);


	//printf("IP : %s\nPort : %d\n",ip,port);

}

static void connectToIp(GtkWidget *widget,GtkBuilder *data){
	GObject *ip_add=gtk_builder_get_object(new_connection_ui,"ip-entry");
	GObject *port_add=gtk_builder_get_object(new_connection_ui,"port-entry");
	char ip[16];
	strcpy(ip,gtk_entry_get_text(GTK_ENTRY(ip_add)));
	char port[8];
	strcpy(port,gtk_entry_get_text(GTK_ENTRY(port_add)));
	unsigned prt;
	int flagIP=0,flagPort=0;
	unsigned char c;
	GdkColor red,green;
	gdk_color_parse("red",&red);
	gdk_color_parse("green",&green);
	if(validateIP(ip)==1)
		flagIP=1;
	
	if(!(sscanf(port,"%u%c",&prt,&c) != 1))
		flagPort=1;
	
	if(flagIP&&flagPort){
		gtk_widget_modify_fg(GTK_WIDGET(ip_add),GTK_STATE_NORMAL,&green);
		gtk_widget_modify_fg(GTK_WIDGET(port_add),GTK_STATE_NORMAL,&red);
		gtk_widget_destroy(new_connection_window);
		//gtk_widget_queue_draw (new_connection_window);
		//printf("IP : %s\nPort : %s\n",ip,port);
		strcpy(taskBuffer,"BYE");	
		sendMessageOverSocket(taskSock,taskBuffer);
		sendMessageOverSocket(cpuSock,taskBuffer);
		sendMessageOverSocket(memSock,taskBuffer);
		sendMessageOverSocket(runSock,taskBuffer);
		close(taskSock);
		close(cpuSock);
		close(memSock);
		close(runSock);
		taskSock=runSock=cpuSock=memSock=-1;
		startConnection(ip,port);
		//Data Valid, start connection.
		
	}else{
		if(!flagIP){
			gtk_widget_modify_fg(GTK_WIDGET(ip_add),GTK_STATE_NORMAL,&red);
		}else{
			gtk_widget_modify_fg(GTK_WIDGET(ip_add),GTK_STATE_NORMAL,&green);
		}
		if(!flagPort){
			gtk_widget_modify_fg(GTK_WIDGET(port_add),GTK_STATE_NORMAL,&red);
		}else{
			gtk_widget_modify_fg(GTK_WIDGET(port_add),GTK_STATE_NORMAL,&green);
		}
	}
}

void createNewConnectionWindow(GtkWidget *widget,gpointer data){
//SETTING UP NEW CONNECTION WINDOW
	gtk_widget_set_sensitive(task_manager_window,FALSE);

	GtkWidget *box;
	GtkWidget *content_area;

	new_connection_window = gtk_dialog_new();
	
	new_connection_ui=gtk_builder_new();
	gtk_builder_add_from_file(new_connection_ui,"new-connection-dialogue.ui",NULL);

	box = gtk_builder_get_object(new_connection_ui,"new-connection-box");
	
	new_connection_okay=gtk_builder_get_object(new_connection_ui,"okay-button");
	g_signal_connect(new_connection_okay,"clicked",G_CALLBACK(connectToIp),new_connection_ui);

	new_connection_cancel=gtk_builder_get_object(new_connection_ui,"cancel-button");
	g_signal_connect(new_connection_cancel,"clicked",G_CALLBACK(closeNewConnectionWindow),new_connection_window);

	content_area = gtk_dialog_get_content_area(GTK_DIALOG(new_connection_window));
	//gtk_widget_reparent(box,GTK_CONTAINER(content_area));
	gtk_container_add(GTK_CONTAINER(content_area), box);

	g_signal_connect(new_connection_window, "destroy", G_CALLBACK(closeNewConnectionWindow), new_connection_window);
	gtk_window_set_title(new_connection_window,"Create New Connection");
	gtk_window_set_skip_taskbar_hint(new_connection_window,TRUE);
	gtk_window_set_resizable(GTK_WINDOW(new_connection_window), FALSE);

	gtk_widget_show_all(new_connection_window);


	
	//gtk_widget_grab_focus(GTK_WIDGET(new_connection_window));
//SETTING UP NEW CONNECTION WINDOW ends.
}

int main(int argc,char *argv[]){

	g_thread_init(NULL);
	gdk_threads_init();

	gdk_threads_enter ();

	gtk_init(&argc,&argv);

//SETTING UP TASK-MANAGER WINDOW
	task_manager_ui=gtk_builder_new();
	gtk_builder_add_from_file(task_manager_ui,"task-manager-ui.ui",NULL);

	task_manager_window = gtk_builder_get_object(task_manager_ui,"task-manager");
	g_signal_connect(task_manager_window,"destroy",G_CALLBACK(shutDown),NULL);

	menubar=gtk_builder_get_object(task_manager_ui,"menubar");
	
	menu_file=gtk_builder_get_object(task_manager_ui,"menuitem-file");
	g_signal_connect(menu_file,"activate",G_CALLBACK(printMenuActivatedData),"Menu-File");

	menu_view=gtk_builder_get_object(task_manager_ui,"menuitem-view");
	g_signal_connect(menu_view,"activate",G_CALLBACK(printMenuActivatedData),"Menu-View");

	menu_options=gtk_builder_get_object(task_manager_ui,"menuitem-options");
	g_signal_connect(menu_options,"activate",G_CALLBACK(printMenuActivatedData),"Menu-Options");

	menu_help=gtk_builder_get_object(task_manager_ui,"menuitem-help");
	g_signal_connect(menu_help,"activate",G_CALLBACK(printMenuActivatedData),"Menu-Help");

	file_newcon=gtk_builder_get_object(task_manager_ui,"menu-file-new");
	g_signal_connect(file_newcon,"activate",G_CALLBACK(createNewConnectionWindow),NULL);

	file_run=gtk_builder_get_object(task_manager_ui,"menu-file-run");
	g_signal_connect(file_run,"activate",G_CALLBACK(printMenuActivatedData),"File-Run");

	file_exit=gtk_builder_get_object(task_manager_ui,"menu-file-exit");
	g_signal_connect(file_exit,"activate",G_CALLBACK(shutDown),NULL);

	options_ontop=gtk_builder_get_object(task_manager_ui,"menu-options-ontop");
	g_signal_connect(options_ontop,"activate",G_CALLBACK(printMenuActivatedData),"Options-OnTop");

	options_minclose=gtk_builder_get_object(task_manager_ui,"menu-options-minclose");
	g_signal_connect(options_minclose,"activate",G_CALLBACK(printMenuActivatedData),"Options-MinClose");

	options_color=gtk_builder_get_object(task_manager_ui,"menu-options-color");
	g_signal_connect(options_color,"activate",G_CALLBACK(printMenuActivatedData),"Options-Color");

	view_refreshnow=gtk_builder_get_object(task_manager_ui,"menu-view-refreshnow");
	g_signal_connect(view_refreshnow,"activate",G_CALLBACK(printMenuActivatedData),"View-Refresh-Now");

	view_refreshspeed=gtk_builder_get_object(task_manager_ui,"menu-view-refreshspeed");
	g_signal_connect(view_refreshspeed,"activate",G_CALLBACK(printMenuActivatedData),"View-Refresh-Speed");

	help_about=gtk_builder_get_object(task_manager_ui,"menu-help-about");
	g_signal_connect(help_about,"activate",G_CALLBACK(printMenuActivatedData),"Help-About");

	gtk_widget_show_all(task_manager_window);

	createNewConnectionWindow(NULL,NULL);

	gtk_main();
	gdk_threads_leave ();

	return 0;
}

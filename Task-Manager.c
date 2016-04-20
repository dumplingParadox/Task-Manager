/*
TO COMPILE :
	 gcc Task-Manager.c -o Task-Manager `pkg-config --cflags --libs gtk+-3.0`

*/

#include<gtk/gtk.h>
#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>

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
//Variable for Task-Manager ends.

//GLOBAL DECLARATIONS ENDS.


//SOCKETS START

void Die(char *mess) { perror(mess); return; }

void setupSocket(char *ip,int port){
	printf("Setting up socket.\n");
	int sock;
	struct sockaddr_in echoserver;
	char buffer[128]="0";
	unsigned int echolen;
	int received = 0;
            
	/* Create the TCP socket */
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		Die("Failed to create socket");
	}

	/* Construct the server sockaddr_in structure */
	memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
	echoserver.sin_family = AF_INET;                  /* Internet/IP */
	echoserver.sin_addr.s_addr = inet_addr(ip);  /* IP address */
	echoserver.sin_port = htons(port);       /* server port */
	/* Establish connection */
	if (connect(sock,(struct sockaddr *) &echoserver,sizeof(echoserver)) < 0) {
		Die("Failed to connect with server");
		return;
	}

	/* Send the word to the server */
	strcpy(buffer,"whoami | xargs top -b -n 1 -o -COMMAND -u | awk \'{printf \"%-s %6s %-4s %-4s %-4s\\n\",$NF,$1,$9,$10,$2}\'");

	//01 COMMANDS [ Get tasks ] : whoami | xargs top -b -n 1 -o -COMMAND -u | awk '{if(NR>7)printf "%-s %6s %-4s %-4s %-4s\n",$NF,$1,$9,$10,$2}'
	//02 COMMANDS [ Get CPU% Usage ] : top -bn1 | grep "Cpu(s)" | sed "s/.*, *\([0-9.]*\)%* id.*/\1/" | awk '{print $1"%"}'	
	//03 COMMANDS [ Get Mem% Usage ] : free -m | grep Mem | awk '{printf("%0.1f%s",$3/$2*100,"%")}'
	
	if (send(sock, buffer, 128, 0) != 128) {
		Die("Mismatch in number of sent bytes");
		return;
	}

	/* Receive the word back from the server */
	while (strcmp(buffer,"exit")!=0) {
		int bytes = 0;
		if ((bytes = recv(sock, buffer, 128, 0)) < 1) {
			Die("Failed to receive bytes from server");
			return;
		}
		received += bytes;
		buffer[bytes] = '\0';        /* Assure null terminated string */
		fprintf(stdout, buffer);
		fprintf(stdout, "\n");
	}

	fprintf(stdout, "\n");
	close(sock);
	return;
}

//SOCKET ENDS

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

void startConnection(char *ip,int port){

	//printf("IP : %s\nPort : %d\n",ip,port);
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

	setupSocket(ip,port);
	
	closeNewConnectionWindow(NULL,loading_connection_window);
}

static void connectToIp(GtkWidget *widget,GtkBuilder *data){
	GObject *ip_add=gtk_builder_get_object(new_connection_ui,"ip-entry");
	GObject *port_add=gtk_builder_get_object(new_connection_ui,"port-entry");
	char *ip=gtk_entry_get_text(GTK_ENTRY(ip_add));
	char *port=gtk_entry_get_text(GTK_ENTRY(port_add));
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
		startConnection(ip,prt);
		gtk_widget_destroy(new_connection_window);
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

	gtk_init(&argc,&argv);

//SETTING UP TASK-MANAGER WINDOW
	task_manager_ui=gtk_builder_new();
	gtk_builder_add_from_file(task_manager_ui,"task-manager-ui.ui",NULL);

	task_manager_window = gtk_builder_get_object(task_manager_ui,"task-manager");
	g_signal_connect(task_manager_window,"destroy",G_CALLBACK(gtk_main_quit),NULL);

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
	g_signal_connect(file_exit,"activate",G_CALLBACK(gtk_main_quit),NULL);

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

	return 0;
}

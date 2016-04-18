/*
TO COMPILE :
	 gcc Task-Manager.c -o Task-Manager `pkg-config --cflags --libs gtk+-3.0`

*/

#include <gtk/gtk.h>

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
	GObject *window,*menubar,*menu_file,*menu_view,*menu_options,*menu_help;
	GObject *file_run,*file_exit;
	GObject *options_ontop,*options_minclose,*options_color;
	GObject *view_refreshnow,*view_refreshspeed;
	GObject *help_about;
//Variable for Task-Manager ends.

//GLOBAL DECLARATIONS ENDS.


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

void startConnection(char *ip,char *port){
	printf("IP : %s\n",ip);
	loading_connection_window = gtk_builder_get_object(new_connection_ui,"loading-connection-dialogue");
	g_signal_connect(loading_connection_window,"destroy",G_CALLBACK(gtk_main_quit),NULL);
	
	loading_connection_cancel=gtk_builder_get_object(new_connection_ui,"cancel-button");
	g_signal_connect(loading_connection_cancel,"clicked",G_CALLBACK(gtk_main_quit),NULL);

	loading_connection_spinner=gtk_builder_get_object(new_connection_ui,"loading-spinner");
	gtk_spinner_start(loading_connection_spinner);

	gtk_widget_show_all(loading_connection_window);
	//gtk_widget_destroy(new_connection_window);

}

static void connectToIp(GtkWidget *widget,GtkBuilder *data){
	GtkBuilder *new_connection_ui = data;
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
		gtk_widget_modify_fg(GTK_WIDGET(port_add),GTK_STATE_NORMAL,&green);
		gtk_widget_hide(new_connection_window);
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

int main(int argc,char *argv[]){

	gtk_init(&argc,&argv);

//SETTING UP NEW CONNECTION WINDOW
	new_connection_ui=gtk_builder_new();
	gtk_builder_add_from_file(new_connection_ui,"new-connection-dialogue.ui",NULL);

	new_connection_window = gtk_builder_get_object(new_connection_ui,"new-connection");
	g_signal_connect(new_connection_window,"destroy",G_CALLBACK(gtk_main_quit),NULL);

	new_connection_okay=gtk_builder_get_object(new_connection_ui,"okay-button");
	g_signal_connect(new_connection_okay,"clicked",G_CALLBACK(connectToIp),new_connection_ui);

	new_connection_cancel=gtk_builder_get_object(new_connection_ui,"cancel-button");
	g_signal_connect(new_connection_cancel,"clicked",G_CALLBACK(gtk_main_quit),NULL);

	gtk_widget_show_all(new_connection_window);
//SETTING UP NEW CONNECTION WINDOW ends.	

//SETTING UP TASK-MANAGER WINDOW
	/*task_manager_ui=gtk_builder_new();
	gtk_builder_add_from_file(task_manager_ui,"task-manager-ui.ui",NULL);

	window = gtk_builder_get_object(task_manager_ui,"task-manager");
	g_signal_connect(window,"destroy",G_CALLBACK(gtk_main_quit),NULL);

	menubar=gtk_builder_get_object(task_manager_ui,"menubar");
	
	menu_file=gtk_builder_get_object(task_manager_ui,"menuitem-file");
	g_signal_connect(menu_file,"activate",G_CALLBACK(printMenuActivatedData),"Menu-File");

	menu_view=gtk_builder_get_object(task_manager_ui,"menuitem-view");
	g_signal_connect(menu_view,"activate",G_CALLBACK(printMenuActivatedData),"Menu-View");

	menu_options=gtk_builder_get_object(task_manager_ui,"menuitem-options");
	g_signal_connect(menu_options,"activate",G_CALLBACK(printMenuActivatedData),"Menu-Options");

	menu_help=gtk_builder_get_object(task_manager_ui,"menuitem-help");
	g_signal_connect(menu_help,"activate",G_CALLBACK(printMenuActivatedData),"Menu-Help");

	file_run=gtk_builder_get_object(task_manager_ui,"menu-file-run");
	g_signal_connect(file_run,"activate",G_CALLBACK(printMenuActivatedData),"File-Run");

	file_exit=gtk_builder_get_object(task_manager_ui,"menu-file-exit");
	g_signal_connect(file_run,"activate",G_CALLBACK(printMenuActivatedData),"File-Run");

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

	gtk_widget_show_all(window);
	*/

	gtk_main();

	return 0;
}

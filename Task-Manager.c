/*
TO COMPILE :
	 gcc <filename>.c -o <filename> `pkg-config --cflags --libs gtk+-3.0`

*/

#include <gtk/gtk.h>
static void printMenuActivatedData(GtkWidget *widget,gpointer data){
  g_print("\n%s was activated.\n",(gchar *)data);
}

int main(int argc,char *argv[]){
	GtkBuilder *task_manager_ui;
	GObject *window,*menubar,*menu_file,*menu_view,*menu_options,*menu_help;
	GObject *file_run,*file_exit;
	GObject *options_ontop,*options_minclose,*options_color;
	GObject *view_refreshnow,*view_refreshspeed;
	GObject *help_about;

	gtk_init(&argc,&argv);

	task_manager_ui=gtk_builder_new();
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
	gtk_main();

	return 0;
}

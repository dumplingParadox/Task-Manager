*******************
**REMOTE TASK MANAGER**
*******************


============
Introduction
============

A windows look alike Task Manager for Ubuntu based Systems, having a host side and a client side application to fetch and manipulate tasks of another remote Ubuntu system.


=============
Main Features
=============

* Complete information about the tasks of current user.  
* Graphical representation of past usage.  
* Killing tasks with just click.  
* Running new tasks.  
* Parallel connections for better performance.  
* Modern UI.


============
Dependencies
============

**Before proceeding, make sure everything is up to date. [ Run `apt-get update` ]**

Gtk3.0+ [ version 3.10.8-0ubuntu1.6 ] : For UI elements and providing look and feel to the project.  
Installation :  
	`apt-get install libgtk-3-0`

Cairo [ version 1.13.0 ] : For 2D drawings, i.e., graphs.  
Installation :  
	`apt-get install libcairo2-dev`


========
Download
========

**Git dependencies must be resolved for the below function.**

To download the project, simply clone it to your current directory.  

Command :  
	`git clone https://github.com/dumplingparadox/Task-Manager.git`


===================
Running The Project
===================

After resolving the dependencies, if any, either recompile the code files, or run from the executables provided.

To Compile :

	Task Manager Client Side:
		gcc Task-Manager.c -o Task-Manager `pkg-config --cflags --libs gtk+-3.0 gthread-2.0` -w

	Task Manager Host Side :
		gcc Task-Manager-Host.c -o Task-Manager-Host -pthread -w

To Run :

	Task Manager Client Side:
		./Task-Manager

	Task Manager Host Side :
		./Task-Manager-Host

After starting the Task Manager, with Host running on the remote Ubuntu system, enter the system ip [Use ifconfig to get system ip] and port for the Host as **8796** to connect to the remote system.

==========
Known Bugs
==========

* Task Manager crashes on a wireless lan network after a duration.
* After enabling 'Minimise on Close', disabling doesn't work.
* While fetching tasks from host, the UI freezes for a second to five, depending on the connection.
* If IP address unreachable, the UI freezes for a minute.
* On outdated devices, menu bar doesn't show up.
* Running new tasks, creates multiple connection for every task, instead of just one.


=======
Authors
=======

Shivam Vijay  
Lakshay Gupta  
Kamini Pandey  

=======
Licence
=======

Feel free to clone and modify the project as per your needs with proper credits.

***
END
***

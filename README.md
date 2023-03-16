net_disk v1.0

description:

    The first phase of the project has been tentatively completed, and the architecture may undergo major changes as the database and more features are introduced later.

    Subsequent updates will continue to add notes and upload code.

by levi

1. how to use:

    After changing the ip address in the configuration file in the conf, the IP address can be used normally.
    Follow the code prompts, add the specified parameters run to establish the connection between the server and the client.

    command:

        -cd: Used to open the specified file directory on the client

        -ls: Used to list all the files in the file directory and show the file type and size

        -pwd: Used to get the local working directory and display it

        -gets: Used to download files from server to client(support breakpoint resume)

        -puts: Used to upload files from client to server

        -remove: Used to delete specified file

        -mkdir: Used to create an empty directory in the local working directory(path parameter unsupported at this time)

        -rmdir: Used to delete empty directory 

        -touch: Used to create new empty file


2. Code Architecture:

    Server main process: receives command requests from clients and assigns all requests to worker child processes.

    Service terminal process: Uses socketpair to receive tasks from the master process and respond to commands.
    
    Client: sends requests to the server and responds to the request processing result of the server

3. Pending function optimization:

    In process pool mode, it can efficiently respond to connection and task requests from multiple clients but without high concurrency. The main process and child process tasks are not evenly distributed. Later, the code architecture will be refactored and thread pools will be adopted.

    User registration, user login, optimized gets and puts for breakpoint resume and second transmission.
    
    rmdir does not support deleting non-empty directories. You can continue to optimize event_loop function later.
    
    
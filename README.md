# Caster_Project #
----------

# Quick Start #

## Running Environment ##

-  Windows

	run `/env/Redis-7.0.15-Windows-x64/start.bat`

- Linux
	
    run `sudo apt install redis-server`

## Download Source Code ##
	
> 	 git clone https://github.com/KOROyo123/Caster_Project.git

## Build With Cmake ##

>     mkdir build
>     cd build
>     cmake ..
>     make


## Edit Configure  ##

- Edit [ Service_Setting.yml ]

>  	vi build/app/Koro_Caster_Service/conf/Service_Setting.yml


- Edit [ Auth_Verify.yml ]
 
>  	vi build/app/Koro_Caster_Service/conf/Auth_Verify.yml

- Edit [ Caster_Core.yml ]

>  	vi build/app/Koro_Caster_Service/conf/Caster_Core.yml


## Run  ##
	
>     cd build/app/Koro_Caster_Service
>     ./Koro_Caster_Service-x.x.x [-port xxxx -conf conf/] 


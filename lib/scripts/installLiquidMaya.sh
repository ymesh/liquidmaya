#!/bin/sh 
#
### liquidmaya install script
### by cgTobi 




autodetectall()
{
clear
printf "\n\nLiquidMaya Install Script\n\nAutodetecting..."

auto_liquid_path=$(locate -n1 src/liquid.version)
len=${#auto_liquid_path}
((len=$len-19))
auto_liquid_path=$(echo $(echo ${auto_liquid_path:0:$len}))

auto_maya_path=$(locate -n1 mayald)
len=${#auto_maya_path}
((len=$len-11))
auto_maya_path=$(echo $(echo ${auto_maya_path:0:$len}))

auto_rman_path=$(locate -n1 libaqsis.so.0)
len=${#auto_rman_path}
((len=$len-14))
auto_rman_path=$(echo $(echo ${auto_rman_path:0:$len}))

#auto_MEL_script_path=$(echo $MEL_script_path)
auto_MEL_script_path=$(locate -n1 prefs/userPrefs.mel)
len=${#auto_MEL_script_path}
((len=$len-20))
auto_MEL_script_path=$(echo $(echo ${auto_MEL_script_path:0:$len}/scripts))

liquid_path=$auto_liquid_path
maya_path=$auto_maya_path
rman_path=$auto_rman_path
MEL_script_path=$auto_MEL_script_path

get_liquid_version

release_choice=release;
}





showinfo()
{
clear
printf "\n\nLiquidMaya Install Script\n\nAutodetected:"
printf "\n\nLiquidMaya version   :   $liquidversion $release_choice"
printf "\nLiquidMaya Path      :   "$liquid_path
printf "\nMEL-Script Path      :   "$MEL_script_path
printf "\nRenderman Path       :   "$rman_path
printf "\nMaya Path            :   "$maya_path

printf "\n\n\nPlease check if everything is correct."
printf "\n\nIf you want to change something type."
printf "\n\n      1 - LiquidMaya Path\n      2 - Maya Path\n      3 - Renderman Path\n      4 - MEL-Script Path\n      5 - Release"
printf "\n\nOr type >>install<< to install liquidmaya now.\n\n>>"
read autoinstall

if [[ $autoinstall = "install" ]]; then
    clear;start_install;printf "\n\nInstalling liquid $liquidversion $release_choice now\n\n";install_now;exit
elif [[ $autoinstall = "1" ]]; then
    clear;get_liquid_path;showinfo;
elif [[ $autoinstall = "2" ]]; then
    clear;get_maya_path;showinfo;
elif [[ $autoinstall = "3" ]]; then
    clear;get_rman_path;showinfo;
elif [[ $autoinstall = "4" ]]; then
    clear;get_MEL_path;showinfo;
elif [[ $autoinstall = "5" ]]; then
    clear;release_install;showinfo;
elif [[ ! $autoinstall = "install" ]]; then
    echo "Quiting installation";exit;
fi

echo "Exit"
exit
}






### Get liquidmaya source path
get_liquid_path()
{
printf "\nAutodetected the liquid binarys in \n $auto_liquid_path\n"
printf "\nHit Enter if the path is correct, \nif not just enter your liquidmaya path (Example: /data/liquidmaya) :\n"
read liquid_path

if [[ $liquid_path = "" ]];then
    liquid_path=$auto_liquid_path;
    echo $liquid_path;
else
	if [ ! -d "$liquid_path" ];then
	    printf "$liquid_path does not exist.\a\n"; 
	    get_liquid_path
	   fi

	if [ ! -e "$liquid_path/src/liquid.version" ];then
	    printf "\nversionINFO does not exist.\a\n"; 
	    get_liquid_path
	   fi
   fi
}




### Get Maya path
get_maya_path()
{
printf "\nAutodetected Maya in \n $auto_maya_path\n"
printf "\nHit Enter if the path is correct, \nif not just enter your Maya path (Example: /usr/aw/maya) :\n"
read maya_path

if [[ $maya_path = "" ]];then
    maya_path=$auto_maya_path;
    echo $maya_path;
else
	if [ ! -d "$maya_path" ];then
	    printf "$maya_path does not exist.\a"; 
	    get_maya_path
	   fi

	if [ ! -e "$maya_path/bin/mayald" ];then
	    printf "\nMaya directory not valid.\a\n"; 
	    get_maya_path
	   fi
    fi
}




### Get Renderman library path
get_rman_path()
{
printf "\nAutodetected Aqsis librarys in \n $auto_rman_path\n"
printf "\nHit Enter if the path is correct, \nif not just enter Aqsis' library path (Example: /usr/local/aqsis/lib) :\n"
read rman_path

if [[ $rman_path = "" ]]
then
    rman_path=$auto_rman_path;
    echo $rman_path;
else
	if [ ! -d "$rman_path" ];then
	    printf "$rman_path does not exist.\a\n"; 
	    get_rman_path
	   fi
	
	if [ ! -e "$rman_path/libaqsis.so.0.0.0" ];then
	    printf "Library path not valid.\a"; 
	    get_rman_path
	   fi
    fi
}




### Get Maya MEL script path
get_MEL_path()
{
printf "\nAutodetected MEL script path in \n $MEL_script_path\n"
printf "\nEnter your MEL script path (Example: /home/foo/maya/scripts) :\n"
read MEL_script_path

if [[ $MEL_script_path = "" ]];then
    MEL_script_path=$auto_MEL_script_path;
    echo $MEL_script_path;
fi
if [ ! -d "$MEL_script_path" ];then
    printf "$MEL_script_path does not exist.\a"; 
    get_MEL_path
   fi
}




### Get liquidmaya version
get_liquid_version()
{
liquidversion=$(cat $liquid_path/src/liquid.version)
len=${#liquidversion}
((len=$len-2))
liquidversion=$(echo liquid$(echo ${liquidversion:1:$len}))
}




### Ask before install
start_install()
{
if [ -d $liquid_path/$liquidversion/bin/redhat/$release_choice ]; then
    cd $liquid_path/$liquidversion/bin/redhat/$release_choice;
    echo "Source is in $liquid_path/$liquidversion/bin/redhat/$release_choice"
elif [ -d $liquid_path/bin/redhat/$release_choice ]; then
    cd $liquid_path/bin/redhat/$release_choice;
    echo "Source is in $liquid_path/bin/redhat/$release_choice"
else
    echo "An error occurred";
    exit
fi
}




### Debug version
release_install()
{
printf "\n\nIf you want to install the LiquidMaya debug version type debug\n"
read release_choice

if [[ $release_choice = "debug" ]] 
then
    release_choice=debug;
else
    release_choice=release;
  fi
}




### Installation script
install_now()
{
cp -v $liquid_path/mel/*.mel $MEL_script_path 

#cd $liquid_path/$liquidversion/bin/redhat/$release_choice 

printf "\nEnter root "
su -c " 

#cp -v $liquid_path/mel/*.mel $MEL_script_path

cp -v liquid $maya_path/bin/; 
 
cp -v liquid.so $maya_path/bin/plug-ins/; 
 
ln -s $rman_path/libaqsis.so.0.0.0 $maya_path/lib/libaqsis.so.0; 
ln -s $rman_path/libaqsistypes.so.0.0.0 $maya_path/lib/libaqsistypes.so.0; 
ln -s $rman_path/libargparse.so.0.0.0 $maya_path/lib/libargparse.so.0; 
ln -s $rman_path/libcodegenvm.so.0.0.0 $maya_path/lib/libcodegenvm.so.0; 
ln -s $rman_path/libddmsimple.so.0.0.0 $maya_path/lib/libcodegenvm.so.0; 
ln -s $rman_path/libddmsimple.so.0.0.0 $maya_path/lib/libddmsimple.so.0; 
ln -s $rman_path/libddmsock.so.0.0.0 $maya_path/lib/libddmsimple.so.0; 
ln -s $rman_path/libddmsock.so.0.0.0 $maya_path/lib/libddmsock.so.0; 
ln -s $rman_path/libdd.so.0.0.0 $maya_path/lib/libdd.so.0; 
ln -s $rman_path/libri2rib.so.0.0.0 $maya_path/lib/libri2rib.so.0; 
ln -s $rman_path/librib2ri.so.0.0.0 $maya_path/lib/librib2ri.so.0; 
ln -s $rman_path/librib2.so.0.0.0 $maya_path/lib/librib2.so.0; 
ln -s $rman_path/librib2stream.so.0.0.0 $maya_path/lib/librib2stream.so.0; 
ln -s $rman_pathb/libshaderexecenv.so.0.0.0 $maya_path/lib/libshaderexecenv.so.0; 
ln -s $rman_path/libshadervm.so.0.0.0 $maya_path/lib/libshadervm.so.0; 
ln -s $rman_path/libslparse.so.0.0.0 $maya_path/lib/libslparse.so.0; 
ln -s $rman_path/libslpp.so.0.0.0 $maya_path/lib/libslpp.so.0; 
ln -s $rman_path/libslxargs.so.0.0.0 $maya_path/lib/libslxargs.so.0; 

"

printf "\nLiquidMaya $liquidversion $release_choice has been successfully installed.\n\n"
}

autodetectall
showinfo


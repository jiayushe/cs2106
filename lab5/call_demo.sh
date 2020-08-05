ARG="input1.in"
P=3

if [ $# -eq 2 ]
then
	ARG=$1
    P=$2
fi

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.
echo "Calling demo with $ARG $P"
./demo $ARG $P

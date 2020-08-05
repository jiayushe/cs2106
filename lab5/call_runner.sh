ARG="input1.in"

if [ $# -eq 1 ]
then
	ARG=$1
fi

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.
echo "Calling runner with $ARG"
./runner < $ARG

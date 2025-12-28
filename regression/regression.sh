array=( test
        BFS1
        MST2
        LR1
        LR2)

for dir in "${array[@]}"
do
    log_file="bin/log.txt";
    tput setaf 3
    cd $dir;
    tput setaf 7
    mkdir -p bin
    make clean && make
    echo ""
    cd ..
done

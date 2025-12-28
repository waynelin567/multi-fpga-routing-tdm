# This script runs all regression in parallel
./regression.sh &
./.slow04.sh &
./.slow05.sh &
./.slow06.sh &

wait

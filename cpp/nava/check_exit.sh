command
status=$?
 
## 1. Run the date command ##
cmd="./main"
$cmd
 
## 2. Get exist status  and store into '$status' var ##
status=$?
 
## 3. Now take some decision based upon '$status' ## 
[ $status -eq 0 ] && echo "$cmd command was successful" || echo "$cmd failed with $status"

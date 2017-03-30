while read line
do
    if [[ "$line" = *"'s New password:" ]] || \
       [[ "$line" = "Enter the new password again:" ]]
   then
       echo "Abcd!234"
   else
       echo ">>> $line"
   fi
done

#!/bin/bash

database_path='./address-book-database.csv'
tmp_file='./tmp'

if (( $# == 1 ))
then
	if [[ $1 = 'view' ]]
	then
		head -n 1 $database_path > $tmp_file
		tail --lines +2 $database_path |  sort -d -t ',' -k 4 | cat $tmp_file - | column -t -s ','
		rm ./tmp
	fi
	
	if [[ $1 = 'insert' ]]
	then
		echo -n 'Name: '
		read input
		entry="$input,"
		echo -n 'Surname: '
		read input
		entry+="$input,"
		echo -n 'Phone: '
		read input
		entry+="$input,"
		echo -n 'Mail: '
		read input
		if [[ -z $(cut -d ',' -f 4 $database_path | grep -n -x $input) ]]
		then
			entry+="$input,"
			echo -n 'City: '
			read input
			entry+="$input,"
			echo -n 'Address: '
			read input
			entry+="$input"
			echo $entry >> $database_path
			echo 'Added'
		else
			echo 'Invalid record: email already exists'
		fi
	fi
fi

if (( $# == 2 ))
then
	if [[ $1 = 'search' ]]
	then
		result=$(tail -n +2 $database_path | grep $2 )
		if [[ -n "$result" ]]
		then
			c=1;
			n=$(echo "$result" | wc -l )
			while (( c<=n ))
			do
				entry=$(echo "$result" | head -n $c | tail -1)
				echo -n 'Name: '
				echo $entry | cut -d ',' -f 1
				echo -n 'Surname: '
				echo $entry | cut -d ',' -f 2
				echo -n 'Phone: '
				echo $entry | cut -d ',' -f 3
				echo -n 'Mail: '
				echo $entry | cut -d ',' -f 4
				echo -n 'City: '
				echo $entry | cut -d ',' -f 5
				echo -n 'Address: '
				echo $entry | cut -d ',' -f 6
				if (( c!=n ))
				then
					echo ''
				fi
				c=$(( $c + 1 ))
			done
		else
			echo 'Not Found'
		fi
	fi

	if [[ $1 = 'delete' ]]
	then
		email=$2
		row_number=$(cut -d ',' -f 4 $database_path | grep -n -x $email | cut -d ':' -f 1)
		if [[ -n $row_number ]]
		then
			head -n $(( row_number-1 )) $database_path > $tmp_file
			tail -n +$(( row_number+1 )) $database_path >> $tmp_file
			cat $tmp_file > $database_path
			rm $tmp_file
			echo 'Deleted'
		else
			echo 'Cannot find any record'
		fi
		 
		
	fi
fi



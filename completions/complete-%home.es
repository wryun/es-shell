fn %complete-%home _ word {
	result $word^<={~~ `` \n {awk -F: '{print $1}' /etc/passwd} $word^*}
}


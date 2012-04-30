
mailq | grep frozen | awk '{print $3}' | xargs exim -v -M 

var=0 
for i in *; do
  var=$(($var + 1))
  mkdir -p ~/pictures_converted/$var/ 
  pdfimages -j $i ~/pictures_converted/$var/ 
  mv ~/pictures_converted/$var/* ~/pictures_converted/$(basename $i .pdf).jpg 
done 



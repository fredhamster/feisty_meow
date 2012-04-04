ssh -i $HOME/.ssh/id_dsa_fred -2 -N -v -L "*:5908:localhost:5908" fred@khandroma.cs.virginia.edu &>~/.tmp/zz_tunnel_khandro_w7.log &
ssh -i $HOME/.ssh/id_dsa_fred -2 -N -v -L "*:4040:localhost:8080" fred@khandroma.cs.virginia.edu &>~/.tmp/zz_tunnel_khandro_jenkins.log &


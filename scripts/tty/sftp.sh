#!/bin/bash

# a wrapper for the file transfers using secure shell.
\sftp -i $HOME/.ssh/id_dsa_fred $*


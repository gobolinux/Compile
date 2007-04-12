#!/bin/bash

. /System/Settings/bashrc

export PATH=$PATH:.

# function to start one instance of gpg-agent for current user
#if [ -z "$(echo $(ps -C gpg-agent -o user=) | grep $(whoami))" ]
#then
#   gpg-agent --daemon -s --write-env-file ~/.gnupg/gpg-agent.env 1> /dev/null
#fi
#export $(cat ~/.gnupg/gpg-agent.env)

FROM pho3be/sh4devenvgitpod:latest

USER root

RUN apt-get update && apt-get install -y nano vim clangd-13 clang-format-13 python3-willow

USER dev

RUN mkdir ~/tmp

RUN alias python=python3

RUN echo -e "echo \"$(tput setaf 2)\n\n [  > v<]~      Welcome to the ClassPad SDK ! \n                You can build using make hhk\n\n$(tput sgr0)\"" >> ~/.bashrc
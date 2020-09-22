FROM registry.cn-qingdao.aliyuncs.com/crf/env:2.0

COPY ./output /home/output

RUN mkdir /home/Judged /home/data \
    && mkdir /home/data/data /home/data/spj \
    && mkdir /home/log \
    && useradd -M -u 1234 sexecer \
    && ln -s /bin/javac /usr/bin/ \
    && ln -s /bin/java /usr/bin/ \
    && apt-get -y install libseccomp-dev libseccomp2 seccomp \
    && mv /home/output/* /usr/bin \
    && rm -rf /home/output

WORKDIR /Judged

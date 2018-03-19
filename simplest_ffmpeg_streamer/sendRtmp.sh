#########################################################################
# File Name: sendRtmp.sh
# Author: zhongjihao
# mail: zhongjihao100@163.com
# Created Time: 2017年12月21日 星期四 19时01分21秒
#########################################################################
#!/bin/bash

ffmpeg -re -i cuc_ieschool.flv -vcodec copy -acodec copy -f flv -y rtmp://10.0.142.118:1935/publishlive/zhongjihao/livestream;




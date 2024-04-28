##CLI Codes For streaming wiht Web Camera

#1-Sender CLI Code 

-> gst-launch-1.0 -v ksvideosrc do-stats=TRUE ! videoconvert \
! x264enc tune=zerolatency bitrate=10000000 speed-preset=superfast ! \
rtph264pay ! udpsink port=5000 host=127.0.0.1

#2-Receiver CLI Code

-> gst-launch-1.0 -v udpsrc port=5000 ! \ 
"application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96" ! rtph264depay ! \
h264parse ! avdec_h264 ! videoconvert ! autovideosink sync=false

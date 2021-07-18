### ffmpeg视频转换
ffmpeg -i butterfly.mp4 -vf scale=240:240 input_output.mp4

ffmpeg -i input_output.mp4 -vf "fps=9,scale=-1:240:flags=lanczos,crop=240:in_h:(in_w-240)/2:0" -c:v rawvideo -pix_fmt rgb565be 240_9fps.rgb

ffmpeg -i butterfly.mp4 -vf scale=180:180 input_output.mp4

ffmpeg -i input_output.mp4 -vf "fps=9,scale=-1:180:flags=lanczos,crop=180:in_h:(in_w-180)/2:0" -c:v rawvideo -pix_fmt rgb565be 180_9fps.rgb
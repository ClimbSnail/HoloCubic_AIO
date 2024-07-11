# Arduino uzlib库，用于解压gzip流

用法:

```cpp
size_t size = stream->available(); // 还剩下多少数据没有读完？
uint8_t inbuff[size]; // 准备一个数组来装流数据，有多少装多少
stream->readBytes(inbuff, size); // 将http流数据写入inbuff中
uint8_t *outbuf=NULL; //解压后的输出流
uint32_t outsize=0; // 解压后多大？在调用解压方法后会被赋值。
// 调用解压函数
int result=ArduinoUZlib::decompress(inbuff, size, outbuf,outsize);
// 输出解密后的数据到控制台。
Serial.write(outbuf,outsize);
```
可以看看example里面的示例

# 参考
1. https://github.com/pfalcon/uzlib



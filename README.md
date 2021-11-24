# reader-writer-semaphore

semaphoreの練習。

```
SiTCP機器 --- reader --- writer
```

とデータが流れる。

readerはソケットを使ってSiTCP機器からデータを読む。
writerはデータをハードディスクに書く。

ハードディスクに書くときにときどきwrite()が遅いことがある。
その間、転送レートが落ちるのを防ぐためにreaderとwriterの
間にバッファを設けている。バッファの構造は

```
#define NBUFF 4096
#define BUFFSIZE 32*1024

typedef struct {
    struct {
        char data[BUFFSIZE];
        ssize_t n;
    } buff[NBUFF];
    sem_t n_empty, n_stored, file_preparation;
} shared_struct;
```

としている。バッファ数4096は必要なら適当に大きくする。

``data[BUFFSIZE]``に読んだデータをいれておく。
``ssize_t n``にはそのバッファに入っているデータバイト数を
いれる（ ここでは例題なのでreaderではreadn()でかならず
BUFFSIZEバイト読むようにしているが、毎回読むバイト数が
変わる場合にも対応できるようにするために``ssize_t n``を
構造体メンバーにいれておいた）。

バッファがいっぱいになった場合にはreaderはexit()するように
している。

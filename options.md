# オプション

inetutils ping と macOS の ping のオプションが折衷されている。

## フラグを立てるもの

### `-v`, `--verbose`

出力が多弁(verbose)になる。
主にパケット受信時。

```
root@0c2c12aecb07:/home# ./ft_ping -c1  google.com
PING google.com (142.250.198.14): 56 data bytes
64 bytes from 142.250.198.14: icmp_seq=0 ttl=62 time=16.405 ms
--- google.com ping statistics ---
1 packets transmitted, 1 packets received, 0% packet loss
round-trip min/avg/max/stddev = 16.405/16.405/16.405/0.000 ms
root@0c2c12aecb07:/home#
root@0c2c12aecb07:/home# ./ft_ping -c1 -v google.com
PING google.com (142.250.198.14): 56 data bytes, id = 0x0392 # ちょっと増える
64 bytes from 142.250.198.14: icmp_seq=0 ttl=62 time=7.669 ms
--- google.com ping statistics ---
1 packets transmitted, 1 packets received, 0% packet loss
round-trip min/avg/max/stddev = 7.669/7.669/7.669/0.000 ms
root@0c2c12aecb07:/home#
```

### `-n`, `--numeric`

受信データグラムの IP アドレスを resolve しない。\
(そもそも課題で受信パケットに対する resolve が禁止されているのでほぼ死にオプションな気がする。)

### `-r`, `--ignore-routing`

ping 送信時にルーティングをしない。\
結果として, 送信するホストに直接接続されている宛先にしか到達しなくなる。

```
# localhost には送信できる
root@0c2c12aecb07:/home# ./ft_ping -c1 -r localhost
PING localhost (127.0.0.1): 56 data bytes
64 bytes from 127.0.0.1: icmp_seq=0 ttl=64 time=0.135 ms
--- localhost ping statistics ---
1 packets transmitted, 1 packets received, 0% packet loss
round-trip min/avg/max/stddev = 0.135/0.135/0.135/0.000 ms
root@0c2c12aecb07:/home#

# google.com には送信できない
root@0c2c12aecb07:/home# ./ft_ping -c1 -r google.com
PING google.com (142.250.198.14): 56 data bytes
ping: sending packet: Network is unreachable
--- google.com ping statistics ---
0 packets transmitted, 0 packets received,
root@0c2c12aecb07:/home#
```

### `-x`, `--hexdump`

受信バッファの先頭から最大 128 オクテットを 16 進ダンプする。

```
root@0c2c12aecb07:/home# ./ft_ping -c1 -x google.com
PING google.com (142.250.198.14): 56 data bytes
[received message] dumping 84 of 84 bytes
000000:  4500 0054 17a4 0000 3e01 63e3 8efa c60e
0x0010:  ac17 0002 0000 dab9 0393 0000 a187 cf64
0x0020:  0000 0000 3035 0300 0000 0000 0001 0203
0x0030:  0405 0607 0809 0a0b 0c0d 0e0f 1011 1213
0x0040:  1415 1617 1819 1a1b 1c1d 1e1f 2021 2223
0x0050:  2425 2627
64 bytes from 142.250.198.14: icmp_seq=0 ttl=62 time=7.597 ms
--- google.com ping statistics ---
1 packets transmitted, 1 packets received, 0% packet loss
round-trip min/avg/max/stddev = 7.597/7.597/7.597/0.000 ms
root@0c2c12aecb07:/home#
```

### `-X`, `--hexdump-sent`

送信バッファの先頭から最大 128 オクテットを 16 進ダンプする。

```
root@0c2c12aecb07:/home# ./ft_ping -c1 -X google.com
PING google.com (142.250.198.14): 56 data bytes
[sent message] dumping 64 of 64 bytes
000000:  0800 d3ef 0394 0000 bf87 cf64 0000 0000
0x0010:  0bfe 0800 0000 0000 0001 0203 0405 0607
0x0020:  0809 0a0b 0c0d 0e0f 1011 1213 1415 1617
0x0030:  1819 1a1b 1c1d 1e1f 2021 2223 2425 2627
64 bytes from 142.250.198.14: icmp_seq=0 ttl=62 time=13.330 ms
--- google.com ping statistics ---
1 packets transmitted, 1 packets received, 0% packet loss
round-trip min/avg/max/stddev = 13.330/13.330/13.330/0.000 ms
root@0c2c12aecb07:/home#
```

### `-?`, `--help`

ping 送受信をせず, ヘルプを表示して終了する。

### `-f`

flood モード。管理者権限が必要。

## 整数値を取るもの

### `-c`, `--count=NUMBER`

ping セッションにおいて, ping を`NUMBER`回送信した後, ping の送信を停止する。デフォルトは無制限。\
送信した回数分の pong を受信し次第, ping セッションを終了する。

```
root@0c2c12aecb07:/home# ./ft_ping -c1  google.com
PING google.com (142.250.198.14): 56 data bytes
64 bytes from 142.250.198.14: icmp_seq=0 ttl=62 time=7.285 ms
--- google.com ping statistics ---
1 packets transmitted, 1 packets received, 0% packet loss
round-trip min/avg/max/stddev = 7.285/7.285/7.285/0.000 ms
root@0c2c12aecb07:/home#
root@0c2c12aecb07:/home# ./ft_ping -c2  google.com
PING google.com (142.250.198.14): 56 data bytes
64 bytes from 142.250.198.14: icmp_seq=0 ttl=62 time=10.935 ms
64 bytes from 142.250.198.14: icmp_seq=1 ttl=62 time=11.840 ms
--- google.com ping statistics ---
2 packets transmitted, 2 packets received, 0% packet loss
round-trip min/avg/max/stddev = 10.935/11.387/11.840/0.205 ms
root@0c2c12aecb07:/home#
```

### `-s`, `--size`

ping データグラムの ICMP ペイロード部のサイズを指定する。デフォルトは 56。

`sizeof(struct timeval)`以上の値が指定された場合はペイロード部にタイムスタンプが埋め込まれ,\
それを使って往復時間(RTT = Round Trip Time)を計測する。

```
root@0c2c12aecb07:/home# ./ft_ping -c1 -s0 -X google.com
PING google.com (142.250.198.14): 0 data bytes
[sent message] dumping 8 of 8 bytes
000000:  0800 f45b 03a4 0000
8 bytes from 142.250.198.14: icmp_seq=0 ttl=62
--- google.com ping statistics ---
1 packets transmitted, 0 packets received, 100% packet loss
root@0c2c12aecb07:/home#
root@0c2c12aecb07:/home# ./ft_ping -c1 -s10 -X google.com
PING google.com (142.250.198.14): 10 data bytes
[sent message] dumping 18 of 18 bytes
000000:  0800 e044 03a2 0000 0001 0203 0405 0607
0x0010:  0809
18 bytes from 142.250.198.14: icmp_seq=0 ttl=62
--- google.com ping statistics ---
1 packets transmitted, 0 packets received, 100% packet loss
root@0c2c12aecb07:/home#
root@0c2c12aecb07:/home# ./ft_ping -c1 -s16 -X google.com
PING google.com (142.250.198.14): 16 data bytes
[sent message] dumping 24 of 24 bytes
000000:  0800 aece 03a3 0000 4c8c cf64 0000 0000
0x0010:  1b9d 0e00 0000 0000
24 bytes from 142.250.198.14: icmp_seq=0 ttl=62 time=7.617 ms
--- google.com ping statistics ---
1 packets transmitted, 1 packets received, 0% packet loss
round-trip min/avg/max/stddev = 7.617/7.617/7.617/0.000 ms
root@0c2c12aecb07:/home#
```

### `-l`, `--preload=NUMBER`

ping セッション開始時に, `NUMBER`個の ping データグラムを間髪入れずに送信する。デフォルトは 0。

### `-w`, `--timeout=N`

ping セッションのタイムアウトを秒単位で指定する。デフォルトは無制限。

```
root@0c2c12aecb07:/home# time ./ft_ping -w1 google.com
PING google.com (142.250.198.14): 56 data bytes
64 bytes from 142.250.198.14: icmp_seq=0 ttl=62 time=9.034 ms
--- google.com ping statistics ---
1 packets transmitted, 1 packets received, 0% packet loss
round-trip min/avg/max/stddev = 9.034/9.034/9.034/0.000 ms

real    0m1.058s
user    0m0.002s
sys     0m0.008s
root@0c2c12aecb07:/home#
root@0c2c12aecb07:/home#
root@0c2c12aecb07:/home# time ./ft_ping -w2 google.com
PING google.com (142.250.198.14): 56 data bytes
64 bytes from 142.250.198.14: icmp_seq=0 ttl=62 time=11.166 ms
64 bytes from 142.250.198.14: icmp_seq=1 ttl=62 time=10.369 ms
--- google.com ping statistics ---
2 packets transmitted, 2 packets received, 0% packet loss
round-trip min/avg/max/stddev = 10.369/10.768/11.166/0.159 ms

real    0m2.063s
user    0m0.003s
sys     0m0.006s
root@0c2c12aecb07:/home#
```

### `-W`, `--linger=N`

最終 ping 送信後のタイムアウトを秒単位で指定する。デフォルトは 10 秒。

### `-m`, `--ttl=N`

ping データグラムの TTL(Time to Live) 値を指定する。デフォルトは 64。

```
root@0c2c12aecb07:/home# time ./ft_ping -c1 -m1 google.com
PING google.com (142.250.198.14): 56 data bytes
92 bytes from 172.23.0.1: Time to live exceeded
--- google.com ping statistics ---
1 packets transmitted, 0 packets received, 100% packet loss

real    0m0.021s
user    0m0.000s
sys     0m0.009s
root@0c2c12aecb07:/home# time ./ft_ping -c1 -m2 google.com
PING google.com (142.250.198.14): 56 data bytes
92 bytes from 192.168.65.5: Time to live exceeded
--- google.com ping statistics ---
1 packets transmitted, 0 packets received, 100% packet loss

real    0m0.018s
user    0m0.003s
sys     0m0.005s
root@0c2c12aecb07:/home# time ./ft_ping -c1 -m3 google.com
PING google.com (142.250.198.14): 56 data bytes
64 bytes from 142.250.198.14: icmp_seq=0 ttl=62 time=10.582 ms # 本来到達してしまうのはおかしい。
--- google.com ping statistics ---
1 packets transmitted, 1 packets received, 0% packet loss
round-trip min/avg/max/stddev = 10.582/10.582/10.582/0.000 ms

real    0m0.021s
user    0m0.000s
sys     0m0.004s
root@0c2c12aecb07:/home#
```

### `-T`, `--tos=NUM`

ping データグラムの TOS(Type of Service) 値を指定する。デフォルトは 0。

## 文字列を取るもの

### `--ip-timestamp=FLAG`

ping データグラムの IP ヘッダに [IP タイムスタンプ](https://datatracker.ietf.org/doc/html/rfc791#page-22)用のオプションを指定する。\
IP ヘッダの指定領域に, 通過したホストごとに通過時刻情報(+α)を記録していく。

`FLAG`には`tsonly`または`tsaddr`を指定する:

- `tsonly` 通過時刻のみを記録する。
- `tsaddr` 通過時刻とホストのアドレスを組で記録する。

```
root@0c2c12aecb07:/home# ./ft_ping -c1 --ip-timestamp tsonly localhost
PING localhost (127.0.0.1): 56 data bytes
64 bytes from 127.0.0.1: icmp_seq=0 ttl=64 time=0.223 ms
TS:     43652903 ms
        43652903 ms
        43652903 ms
        43652903 ms

--- localhost ping statistics ---
1 packets transmitted, 1 packets received, 0% packet loss
round-trip min/avg/max/stddev = 0.223/0.223/0.223/0.000 ms
root@0c2c12aecb07:/home#
root@0c2c12aecb07:/home#
root@0c2c12aecb07:/home#
root@0c2c12aecb07:/home# ./ft_ping -c1 --ip-timestamp tsaddr localhost
PING localhost (127.0.0.1): 56 data bytes
64 bytes from 127.0.0.1: icmp_seq=0 ttl=64 time=0.059 ms
TS:     localhost (127.0.0.1)   43655409 ms
        localhost (127.0.0.1)   43655409 ms
        localhost (127.0.0.1)   43655409 ms
        localhost (127.0.0.1)   43655409 ms

--- localhost ping statistics ---
1 packets transmitted, 1 packets received, 0% packet loss
round-trip min/avg/max/stddev = 0.059/0.059/0.059/0.000 ms
root@0c2c12aecb07:/home#
```

### `-p`

ping データグラムの ICMP ペイロード部の空き領域を埋めるためのパターンを指定する。

```
root@0c2c12aecb07:/home# ./ft_ping -c1 -s20 -X google.com
PING google.com (142.250.198.14): 20 data bytes
[sent message] dumping 28 of 28 bytes
000000:  0800 9632 039b 0000 8e8b cf64 0000 0000
0x0010:  f93d 0500 0000 0000 0001 0203
# 何も指定しない場合は 後ろ4バイトが 00 01 02 03 になる
28 bytes from 142.250.198.14: icmp_seq=0 ttl=62 time=7.565 ms
--- google.com ping statistics ---
1 packets transmitted, 1 packets received, 0% packet loss
round-trip min/avg/max/stddev = 7.565/7.565/7.565/0.000 ms
root@0c2c12aecb07:/home#
root@0c2c12aecb07:/home#
root@0c2c12aecb07:/home# ./ft_ping -c1 -s20 -X -p abcd google.com
PING google.com (142.250.198.14): 20 data bytes
[sent message] dumping 28 of 28 bytes
000000:  0800 93ea 039c 0000 988b cf64 0000 0000
0x0010:  9fed 0100 0000 0000 abcd abcd
# abcd を指定したので後ろ4バイトが ab cd ab cd になる
28 bytes from 142.250.198.14: icmp_seq=0 ttl=62 time=10.345 ms
--- google.com ping statistics ---
1 packets transmitted, 1 packets received, 0% packet loss
round-trip min/avg/max/stddev = 10.345/10.345/10.345/0.000 ms
root@0c2c12aecb07:/home#
```

### `-S`

送信元の IP アドレスを指定値に固定する。

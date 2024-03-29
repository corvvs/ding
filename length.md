# 「長さ」あるいは「データサイズ」について

## 一次情報と二次情報

データの「長さ」と呼ばれるものにはおおまかに 2 種類あると考える:

1. 実際に測定されたもの(**一次情報; primary**)
2. データ自身がそう主張しているもの(**二次情報; secondary**)

### 一次情報の例

- 送信バッファの長さ
  - 自分でバッファを作ったはずなので, 当然正確な値を知っているはず.
- 受信データのサイズ
  - `recvmsg`の返り値は信じていい.
- (IP ヘッダの最小の長さ)
- (ICMP ヘッダの長さ)
  - 規格上, 変動がない

### 二次情報の例

- IP ヘッダに書かれているヘッダサイズ
  - IHL
- IP ヘッダに書かれているデータグラムサイズ
  - Total Length
- _ICMP Time Exceeded に埋め込まれている_ IP ヘッダのヘッダサイズ
- _ICMP Time Exceeded に埋め込まれている_ IP ヘッダのデータグラムサイズ

## 二次情報には嘘がある

二次情報には適当な値が書かれていることがある。\
たとえば IHL をそのまま信じて ICMP ヘッダの場所を計算すると, バッファオーバーフローになるかもしれない.\
よって二次情報を使う際は, 一次情報に矛盾しないことを厳重に確認する必要がある.

## 三次情報

一次情報・二次情報から算出された値を**三次情報**(tertiary)と呼ぶことにする.

例:

- ICMP データグラムサイズ
  - `IP Total Length - IP IHL * 4`
- ICMP ペイロードサイズ
  - `ICMP データグラムサイズ - ICMPヘッダサイズ`

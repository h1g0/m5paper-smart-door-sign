# M5Paper-Smart-Door-Sign

[English version](README.md)

![このリポジトリのコードを M5PaperS3 で実行し、「Meeting In Progress」を表示している様子。](img/smart_door_sign.png)

電子ペーパーを使用したドアボードです。Googleスプレッドシートを書き換えることで、「取り込み中」や「会議中」などの表示を遠隔で更新できます。

## 特徴

* **遠隔更新:** Google スプレッドシートを書き換えることで、どこからでも表示内容を変更可能。
* **超低消費電力:** M5PaperのDeep Sleepと電子ペーパーを活用し、1回の充電で数週間動作可能。
* **ゼロ電力保持:** バッテリー切れになっても電子ペーパーの表示が残る。
* **ミニマルデザイン:** 日本語/英語フォントに対応した、高コントラストで見やすいUI。

## 必要なハードウェア

* **M5Paper**（または M5PaperS3）
* Wi-Fi 接続環境
* 充電・書き込み用 USB-C ケーブル

## システム構成

1. **Google スプレッドシート:** 表示内容を設定します。小さな表で main/sub のテキストとサイズを管理します。
2. **Google Apps Script (GAS):** 表示内容をJSON APIとして配信します。
3. **M5Paper:** 定期的に起動し、Wi-Fi経由でJSONを取得して電子ペーパーを更新し、再びスリープします。

## インストール

### 1. Google Apps Script の設定

1. 新しい **Google スプレッドシート** を作成する。
2. **A1:C3** に、以下の形式で表を作成する。
3. `拡張機能` > `Apps Script` を開き、`gas/gas.gs` のコードを貼り付ける。
4. **ウェブアプリ** としてデプロイし、アクセス権を **「全員」** に設定する。
5. 発行された **ウェブアプリ URL** をメモしておく。

#### 表の形式

|      | text | size |
|:-----|:-----|:-----|
| main |      |      |
| sub  |      |      |

![スプレッドシートの指定位置にこのように配置してください。](img/table.png)

### 2. ファームウェアの設定

1. Arduino IDE で `main.ino` を開く。
2. 同じタブに `config.h` を作成し、以下を設定する。

```cpp
// WiFi Configuration
const char *SSID = "YOUR_WIFI_SSID";
const char *PASS = "YOUR_WIFI_PASS";

// GAS Configuration
const char *GAS_URL = "YOUR_GAS_URL";

// NTP Configuration
const char* NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = 9 * 3600; // Japan Standard Time (GMT+9). Change as needed.
const int DAYLIGHT_OFFSET_SEC = 0;

// Update interval (minutes)
const int SLEEP_MIN = 10;
```

（`config.h.example` も参照）

1. 依存ライブラリ `M5Unified`、`M5GFX`、`ArduinoJson` をインストールする。
2. M5Paper にコードを書き込む。

## 使い方

* 画面は `SLEEP_MIN` 分ごとに更新されます。
* 表示メッセージを変えるには、Google スプレッドシートの `main` / `sub` 行（`text` と `size` 列）を編集します。
* 更新間の消費電力を抑えるため、デバイスは `M5.Power.timerSleep()` を使ってスリープします。
* 右上の "off" ボタンは Wi-Fi 接続画面でのみ表示されます。
* Wi-Fi 接続画面（最大 10 秒、最短 1 秒表示）で "off" をタッチすると、無期限の電源オフモードに入ります。
  * 再起動するには M5Paper の電源ボタンを押してください。

### 表の入力例

|      | text        | size |
|:-----|:------------|:-----|
| main | Meeting     | 8    |
| sub  | In Progress | 4    |

![作成したスプレッドシートにはこのように文字列と数値を入力します。](img/table-example.png)

## ライセンス

MIT License

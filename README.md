# Udev Sample

## 1. 環境構築
```bash
以下を実施してください。
sudo apt install build-essential
sudp apt install python-pip
python -m pip install pyudev
python -m pip install -U six
apt-get install python-pyudev
apt-get install python-serial
```

## 2. Ｃソースファイルのビルド方法
1. Drivers直下にbuildディレクトリを作成
2. buildディレクトリに移動
3. 「cmake ..」を実行
4. 「make」を実行
5. 「udev_app」が生成される


## 3. 各ソフトウェアの起動
以下の順序で起動してください。
1. ./udev_app
2. python udev_event.py

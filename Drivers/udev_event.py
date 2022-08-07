# -*- coding: utf-8 -*-

import pyudev
import socket
import serial
import serial.tools.list_ports

# IDはよしなに変更すること
VENDER_ID = '1a2b'
PRODUCT_ID = '3c4d'
UNIX_DOMAIN_SOCKET_PATH = "/tmp/udev-socket"

def main():
    # デバイスの接続が検知されたら、socketで通知したい。まずは接続
    s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    s.connect(UNIX_DOMAIN_SOCKET_PATH)

    # まず、起動した時点で必要なデバイス達が、接続されているか
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        print('Special File (Device Name) : ' + p.device)
        print('Vendor ID : ' + str(format(p.vid, '04x')))
        print('Product ID : ' + str(format(p.pid, '04x')))
        print('Serial Number : ' + str(p.serial_number))
        print('Location : ' + p.location)
        print('MAnufacturer : ' + p.manufacturer)
        print('Product Name : ' + p.product)
        print('Interface : ' + str(p.interface))
        print('---')
        if(str(format(p.vid, '04x')) == VENDER_ID and str(format(p.pid, '04x')) == PRODUCT_ID):
            # デバイスファイルを文字列で送信する
            s.send('Add Device!!,' + p.device)
            print('---')

    # udevをモニタリングして、デバイスの抜き差しを監視する
    context = pyudev.Context()
    monitor = pyudev.Monitor.from_netlink(context)
    monitor.filter_by(subsystem='tty')
    monitor.start()


    for device in iter(monitor.poll, None):
        print(device.action)
        print('MODEL  : {0} -> {1}'.format(device.get('ID_MODEL_ID'), device.get('ID_MODEL_FROM_DATABASE')))
        print('VENDOR : {0} -> {1}'.format(device.get('ID_VENDOR_ID'), device.get('ID_VENDOR_FROM_DATABASE')))
        print('SERIAL : {0} -> {1}'.format(device.get('ID_SERIAL_SHORT'), device.get('ID_SERIAL')))
        print('SPECIAL FILE (DEVICE NAME) : {0} ({1})'.format(device.get('DEVNAME'), device.get('DEVTYPE')))
        print('---')

        if(device.get('ID_VENDOR_ID') == VENDER_ID and device.get('ID_MODEL_ID') == PRODUCT_ID):
            # 刺した
            if(device.action == 'add'):
                # デバイスファイルを文字列で送信する
                s.send('Add Device!!,' + device.get('DEVNAME'))
                print('---')
            # 抜いた
            if(device.action == 'remove'):
                # デバイスファイルを文字列で送信する
                s.send('Remove Device!!,' + device.get('DEVNAME'))
                print('---')


if __name__ == "__main__":
    main()
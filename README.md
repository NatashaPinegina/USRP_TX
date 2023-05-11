# USRP_TX
### Программа предназначена для генерации сигналов с заданным типом модуляции (реализован BPSK) и их передачи с помощью USRP

### Сборка
#### Сборка и установка UHD
https://files.ettus.com/manual/page_build_guide.html

Решение возможных проблем:

https://stackoverflow.com/questions/33304828/when-trying-to-use-my-usrp-in-gnu-radio-i-get-a-no-devices-found-for
#### Сборка кода
```
git clone https://github.com/NatashaPinegina/USRP_TX.git
cd USRP_TX
mkdir build
cd build
cmake ../
make
./uhd
```

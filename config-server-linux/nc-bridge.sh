# ncat -lkv 39000 -c 'tee /dev/stderr | ncat 127.0.0.1 39001 | tee /dev/stderr'
ncat -lkv 39000 -c 'tee /dev/stderr | ncat 127.0.0.1 39001'

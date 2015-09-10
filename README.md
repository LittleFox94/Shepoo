# Shepoo
It's sheep poo.

## What is it?
It a collection of programs to store data on multiple servers on the internet. If one of these servers goes offline, nothing of the data can be recovered.

No server holds everything of the data, so a single server-owner can't use the data in a bad way, this is great for the user and great for the server-owner, since he doesn't know what data is stored on his server and thus can't be made liable for it.

If a group of people want to distribute files in a way, they can't be made liable, they can install some kind of kill-switch to one of the servers and delete all the files and metadata this way.

## How does it work?
Currently we only support 3 servers, 2 for each half of the data and one to map the blocks together.

Data is organized in blocks. For any given block, there is a mapping to a block on each server on the shuffle-server. 
Let's say we want to have block 5. We would ask the shuffle-server which block we need to read from server A and server B (these WILL be different blocks). Now we XOR these blocks and get the cleartext.

Since XOR can be attacked with known-plaintext-attacks, we don't use a traditional key. We generate a random byte and XOR it with the data byte, resulting in the cipher-byte. The random- and cipherbytes get stored on the servers.

### Encrypt
```
byte data
byte resultA = bestRandomnessInTheWorld()
byte resultB = data XOR resultA
```
### Decrypt
```
byte dataA
byte dataB

byte data = dataA XOR dataB
```
It's actually quite simple and we currently don't see any problems with this, but we are no cryptographers so please let us know if you see any. Since we use random data as "key", this is an one-time pad, which is, in theory, unbreakable.

## Known problems
* For each block of data you want to download, you actually have to download two (amount of storage servers) blocks. In the current setup we double the amount of data transferred, in later setups this can be even worse.
* If someone grabs all servers at once, he can reconstruct all of the data without problems, so data stored in the blocks should also be encrypted
* Users can theoretically store plain-text blocks on the server.

## What the name?!
pwgen

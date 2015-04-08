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

Since XOR can be attacked with known-plaintext-attacks, we don't use a traditional key. We split each byte in half, giving us a lower and a higher half-byte (nibble). Then we generate a random byte and do the same again. Now we XOR the lower nibble of the random byte with the lower nibble of the data byte and then repeat this for the higher nibbles. At the end, we store the the XORed higher nibble with the random lower nibble on server A and the higher random nibbler with the XORed lower nibble on server B.

### Encrypt
```
byte data
byte random = bestRandomnessInTheWorld()

nibble dataLower = LOWER(data)
nibble dataUpper = UPPER(data)
nibble randomLower = LOWER(data)
nibble randomUpper = UPPER(data)

nibble resultLower = dataLower XOR randomLower
nibble resultUpper = dataUpper XOR randomUpper
 
byte resultA = TOHIGHER(resultUpper) | randomLower
byte resultB = TOHIGHER(randomUpper) | resultLower
```
### Decrypt
```
byte dataA
byte dataB

byte data = dataA XOR dataB
```
It's actually quite simple and we currently don't see any problems with this, but we are no cryptographers so please let us know if you see any. If you know the plaintext of some of the XORed nibbles on one server, you still can't guess the other data as there isn't any repeated key. Since we use random data as "key", this is, in theory, very close to one-time pad, which is, in theory, unbreakable.

## Known problems
* For each block of data you want to download, you actually have to download two (amount of storage servers) blocks. In the current setup we double the amount of data transferred, in later setups this can be even worse.
* If someone grabs all servers at once, he can reconstruct all of the data without problems.
* Users can theoretically store plain-text blocks on the server.

## What the name?!
pwgen

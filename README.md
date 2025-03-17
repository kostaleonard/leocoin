# LeoCoin

This app allows you to mine LeoCoin, the hottest new cryptocurrency.

## How much is LeoCoin worth?

LeoCoin is ~~worthless~~ priceless.

## Quickstart

Cryptocurrencies use cryptographic keys to provide secure transactions.
First, generate a public/private key pair.

```sh
openssl genpkey -algorithm RSA -out private_key.pem
openssl rsa -pubout -in private_key.pem -out public_key.pem
```

We are going to pass these keys to our app container using environment variables.
Environment variables cannot contain newline characters.
base64 encode the key contents to remove newlines and store the result in environment variables or files.

```sh
export LEOCOIN_PRIVATE_KEY=$(base64 -w 0 private_key.pem)
export LEOCOIN_PUBLIC_KEY=$(base64 -w 0 public_key.pem)
```

Now run the container with the following arguments.

* `--init`: Optional. Allows you to pass CTRL-C signals to the app running in the container.
* `--rm`: Optional. Removes the container when you stop it.
* `--env`: Required. Passes the keys to the app.

```sh
docker run --init --rm --env LEOCOIN_PRIVATE_KEY=$LEOCOIN_PRIVATE_KEY --env LEOCOIN_PUBLIC_KEY=$LEOCOIN_PUBLIC_KEY kostaleonard/leocoin
```

You can see the miner computing hashes.
It will display winning hashes in green text, indicating that the peer has mined a new block.

![LeoCoin mining](media/leocoin_mining.gif)

## Peer discovery bootstrap server

The app relies on a bootstrap server to discover other miners.
If you want to stand up your own mining network, you should also run this server.
You can find the executable in the `build` directory in the container, or after building on Windows or Unix with CMake.

## Consensus

The consensus algorithm allows peers to agree on a view of the blockchain, which is a distributed ledger of transactions.
Each peer mines blocks on their own chain.
When they discover a new block, they broadcast the result to the rest of the network.
Peers will exchange blockchains, and the longest blockchain will be the agreed upon view.

The blue text shows when a peer has switched to a longer chain.

![Switch to longer chain](media/switch_to_longer_chain.gif)

## An example with several peers

Here is what the mining process looks like when three peers are participating.
The upper left window is the peer discovery bootstrap server.

![Three miners](media/three_miners.gif)

# rps-protonchain-contract

## Contents

* [Description](#description)
    * [Threat Model](#threat-model)
* [Usage](#usage)
* [Implementation](#implementation)
    * [Registration Phase](#registration-phase)
    * [Commit Phase](#commit-phase)
    * [Reveal Phase](#reveal-phase)
    * [Result Phase](#result-phase)
    * [Helper Functions](#helper-functions)


## Description

This smart contract implements a secure Rock-Paper-Scissors game in c++, ready to be deployed on the [protonchain](https://proton.bloks.io/). The game follows these steps:
1. Two players register and place a bet.
2. Each participant picks a choice. They send the hash of the concatenated string to the contract which stores it.
3. When the two players have committed their choice, it is signed by the some key to reveal what they have played. To do so, they send their choice and signature and key in clear. The contract verifies that the hash of the received input matches the one stored.
4. In each round, when both player have chosen their choice, the contract determines the winner in that round and start new round until one player will win 2 times. If there is a draw, new round is started. When one player wins 2 times, he is the winner and receive the total amount except 1% fee.
5. The game resets and can be played again by new players.

### Threat Model

This contract is *secure* in the sense that a player who has access to the blockchain and its content would still not be able to guess another player's choice. Indeed the contract never stores any of the players' choice in clear, but only the hash of the choice salted with a password only known to the player. Since players cannot change their choice during the reveal phase (after they have both committed their choice), this effectively ensures that an opponent could not cheat by looking at transaction data and playing accordingly.

This assumes that the hash function used is both preimage resistant  and second-preimage resistant. But for the hash used, SHA256, no one has managed to break pre and second image resistance yet.


## Usage

1. Register with function `create()`. You must deposit the game level amount of bet. If the host don't pay anything, he can't play(can't make the choice).

1. Join with function `join()`. Player can join to the created game and he should deposit the amount equal to the first player's deposit amount.

2. Commit a choice with function `makechoice()`. The format of the expected input is `"0xHASH"` where `HASH` is the SHA256 hash of a string `password:choice`. `choice` is an abbreviation of rock, paper and scissors respectively and `password` is a string that should be kept secret.

3. When both players have made the choice, the round is over.If one player doesn't choose his choice within 5 second, the choice of him is selected automatically and checked the round results.

4. If one player wins 2 times in rounds first, he is the winner of the one game and receive the total amount that two players deposit except the 1% fee.
5. If one is disconnected, then the bot will play the game instead of him. He can play that game after he connect in case the game is not closed. The each round has the 5s timeout and is closed automatically whenever the play don't choose his choice. The random choice is selected instead of the play's commit in that case. The game is closed when one player wins 2 rounds first.


## Implementation

### Registration Phase

Anyone can register provided that they're not already registered and that their bet is greater than a fixed minimum, currently 1 finney.

If a player has already been registered, a second player wishing to register must place a bet greater than or equal to the bet of that previous player.
The second player should deposit with the same amount that host deposit. He/she should not deposit smaller amount than other's.
After two players deposit the appropriate amount.  the game starts.
## Commit Phase

When a player has been registered successfully, he/her can play. As described previously, the player provides a SHA256 hash of the concatenation of a choice, and a secret password. The contract stores this hash and nobody except the player has access to the actual choice. Once such a hash has been committed, it cannot be modified.

## Reveal Phase

The reveal phase begins when both player have committed their choice. A player reveals what he had played by sending `password:choice`. The contract then checks if its indeed the choice that was played by hashing it and comparing the result to the stored hash. If they're equal, the first character of the string (`choice`, an integer) is saved and the contract waits for the second player to reveal its choice.
The player can check the game status by calling of the `getstate()` and if the other make the choice,they will reveal the choice automatically.

## Result Phase

When the reveal phase ends, the current round of the game is checked automatically. If there is the winner in that game, the game is ended and prize will be sent to the winner.


## Helper Functions

At any time, players have access to public state variables and helper functions to get information about the state of the game.



Functions available are:
* `create()`: create the game by the host.
* `restart()`: restart the game with the smme opponent.
* `close()`:  close the game with unique game id.
* `join()`:   join to the game that the host created. He should deposit the amount the same as the host depost.
* `makechoice()`: commit the choice of the rps with hash value. If opponet has already made the choice, the contract check the game result and start new round or finish the game when the one wins two rounds first.
* `unlockchoice()`: commit the real choice with player's random password, so the contract check whether the choice is the same as the hash he sent before. If it is different he can't commit his choice and bot will play instead of him after game time out. 

Public state variables are:
* `states`: static variable of the rps game results.


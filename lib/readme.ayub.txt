
hat-trie:
git clone https://github.com/dcjones/hat-trie.git
cd hat-trie
autoreconf -i
./configure --prefix=$HOME/.local
make
make install



Try to configure libraries to be installed in $HOME/.local
./configure --prefix=$HOME/.local
make
make install

If older autoconf/automake used, type:
aclocal
autoconf
automake
./configure --prefix=$HOME/.local
make
make install




JX9, PH7, UNQLITE: Don't use any of them for production. Their code has bugs
which incorrectly parses data.





git clone https://github.com/symisc/PH7.git
git clone https://github.com/symisc/unqlite.git

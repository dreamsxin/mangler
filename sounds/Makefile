
all: mangler-sounds.h

bin2h: bin2h.c
	gcc -Wall -o bin2h bin2h.c

mangler-sounds.h: bin2h channelenter.h channelleave.h login.h logout.h talkend.h talkstart.h
	cat channelenter.h channelleave.h login.h logout.h talkend.h talkstart.h > mangler-sounds.h
	
channelenter.h: bin2h channelenter.raw
	./bin2h channelenter.raw channelenter.h sound_channelenter

channelleave.h: bin2h channelleave.raw
	./bin2h channelleave.raw channelleave.h sound_channelleave

login.h: bin2h login.raw
	./bin2h login.raw login.h sound_login

logout.h: bin2h logout.raw
	./bin2h logout.raw logout.h sound_logout

talkend.h: bin2h talkend.raw
	./bin2h talkend.raw talkend.h sound_talkend

talkstart.h: bin2h talkstart.raw
	./bin2h talkstart.raw talkstart.h sound_talkstart

clean:
	rm -f mangler-sounds.h
	rm -f channelenter.h
	rm -f channelleave.h
	rm -f login.h
	rm -f logout.h
	rm -f talkend.h
	rm -f talkstart.h

distclean: clean
	rm -f bin2h

install: mangler-sounds.h

uninstall:

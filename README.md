# Hello world configurabile per UARM/UMPS

Questa repository contiene un esempio di programma compilabile sia per l'emulatore uMPS2 (https://github.com/tjonjic/umps) che uARM (https://github.com/mellotanica/uARM). 
I due emulatori offrono librerie ROM e dispositivi mappati in memoria molto simili, per cui ottenere un risultato cross-platform e' relativamente semplice. Tramite delle macro `#ifdef` si includono gli header delle rispettive routine ROM e gli indirizzi dei dispositivi (in questo caso, solo il terminale).

A scopo di esempio sono implementati almeno due possibili metodi per la configurazione dell'architettura: make e scons.

## Requisiti

Perche' la compilazione vada a buon fine sono necessari i seguenti pacchetti:

- arm-none-eabi-gcc
- mipsel-linux-gnu-gcc
- uarm (per la compilazione su uarm)
- umps (per la compilazione su umps)
- make (per utilizzare i makefile)
- python-scons (per utilizzare SConstruct)
- python-kconfiglib (per utilizzare SConstruct)

## Make

Molto semplicemente vengono forniti due makefile separati per la compilazione, `uarmmake` e `umpsmake`. Invocando `make uarm` o `make umps2` si procede con la compilazione dell'architettura richiesta.

Dietro le quinte le differenze tra i due makefile sono:

 - utilizzo di un compilatore e di flag di compilazione appropriati
 - compilazione di diverse librerie di base
 - inclusione di diversi header
 - definizione delle macro `TARGET_UMPS` o `TARGET_UARM` per ottenere un comportamento diverso (in questo semplice esempio la cosa si riduce all'includere degli header diversi)

## Scons e Kconfig

Scons e' un build tool alternativo a make. Si tratta sostanzialmente di una libreria Python per la gestione di sorgenti. Invocando il comando `scons` viene eseguito lo script `SConstruct`, analogamente al funzionamento di make.
Usando i parametri `uarm` o `umps` e' possibile differenziare il target a riga di comando in maniera del tutto analoga al funzionamento di make.

```
$ scons umps
$ scons uarm
```

Oltre a questo semplice utilizzo pero' lo script `SConstruct` e' configurato anche per utilizzare il meccanismo di configurazione tipico del kernel Linux, Kconfig.
Kconfig comporta la definizione di menu di configurazione (i file `Kconfig`) che seguono una sintassi specifica (https://www.kernel.org/doc/html/latest/kbuild/kconfig-language.html). Molteplici tool sono poi in grado di leggere questi file e comportarsi di conseguenza o generare degli header che a loro volta influenzino il comportamento dei sorgenti.
Uno di questi strumenti e' `kconfiglib`, una libreria Python che fornisce sia delle interfacce (grafiche e non) per la modifica della configurazione che delle API per la gestione programmatica di quest'ultima. Essendo un tool in Python scons si puo' interfacciare direttamente a queste API, come viene fatto in questo esempio.

Per installare scons e `kconfiglib` si consiglia di appoggiarsi a un environment virtuale:

```
$ virtualenv .env
$ source .env/bin/activate
$ pip install -r requirements.txt
```

A questo punto e' possibile editare la configurazione (specificata dal file `Kconfig`) con il comando `guiconfig` o `menuconfig`. Una volta salvata una nuova configurazione lanciando il comando `scons` senza argomenti questa verra' usata per decidere il target di compilazione.

## Esecuzione

Per l'esecuzione dell'esempio fare riferimento ai manuali di uARM e uMPS2, rispettivamente.
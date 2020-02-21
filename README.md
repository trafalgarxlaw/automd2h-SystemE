# automd2h - TP1
## Manuel
automd2h convertit les fichiers markdown d'une sous-arborescence vers des fichiers HTML

## By me
Utiliser un max d'appel system : opendir, closedir (pour se promenet dans les repertoire de facon recurive)
fork() wait() etc...

**System calls**
- fork()
- wait()

Faudrais lister toutes les fonctions Appel System qu'on pense avoir besoin.

## Synopsis
automd2h [options] fichier...

## Description
automd2h convertit les fichiers au format Markdown en fichiers au format HTML.

Si fichier est un fichier régulier, celui-ci est converti en HTML avec l'outil pandoc. Le fichier HTML est placé dans le même répertoire et a le même nom que le fichier original avec l'extension .html ajoutée. Si l'extension originale du fichier est .md, alors celle-ci est retirée du nom du fichier cible et est remplacée par .html.

**Exemple: automd2h foo.txt bar.md baz/foo.bar.md crée les fichiers foo.txt.html, bar.html et baz/foo.bar.html**

Si fichier est un répertoire, automd2h cherche les fichiers .md de ce répertoire. Chacun de ces fichiers sources est converti et le fichier cible .html est placé dans le même répertoire et a le même nom que le fichier .md source, seule l'extension change.

**Exemple: automd2h baz trouve baz/foo.bar.md et crée le fichier baz/foo.bar.html.**
Option -t
Avec l'option -t. La date de dernière modification des fichiers est utilisée pour savoir s'il faut reconvertir. Si le fichier source est plus récent que le ficher .html cible associé, ou si le fichier .html cible n'existe pas, alors il y a conversion. Si la date est identique ou si le fichier .html cible est plus récent, alors il n'y a pas de conversion.

Option -n
L'option -n désactive l'utilisation de pandoc, à la place, la liste des chemins des fichiers sources à convertir sera affichée (un par ligne).

Combiné avec -n, l'option -t n'affiche que les fichiers sources effectivement à convertir.

Option -r
L'option -r visite les répertoires récursivement et cherche les fichiers dont l'extension est .md pour les convertir.

**Exemple: automd2h -r . va trouver bar.md et baz/foo.bar.md et créer les fichiers bar.html et baz/foo.bar.html. Par contre foo.txt n'est pas considéré, car l'extension n'est pas la bonne.**

Note: dans la recherche récursive, les liens symboliques vers des fichiers .md sont suivis mais les liens symboliques vers des répertoires ne sont pas suivis.

Option -w
Avec l'option -w, automd2h bloque et surveille les modifications des fichiers et des répertoires passés en argument. Lors de la modification d'un fichier source, celui-ci est automatiquement reconverti. Si dans un répertoire surveillé un fichier .md apparait, est modifié, est déplacé ou est renommé, celui-ci aussi est automatiquement converti.

**Exemple: automd2h -w bar.md baz ne fait rien, bloque et attend les modifications des fichiers. Si bar.md ou baz/foo.bar.md est modifié, il sera reconverti, si un nouveau fichier baz/new.md apparait il sera aussi reconverti. Par contre, les modifications ou création de baz/foo.txt ne sont pas considérées car l'extension n'est pas la bonne. De même pour ./foo.md ou baz/subdir/foo.md qui ne sont pas dans un répertoire surveillé.**

Note: En mode surveillance (-w), le programme se ne termine pas et doit être terminé manuellement (avec Ctrl-C par exemple).

Combiné avec -n, l'option -w attend indéfiniment et affiche les fichiers à convertir au lieu d'appeler pandoc.

Combiné avec -r, l'option -w surveille aussi les sous-répertoires. Si dans un répertoire surveillé un sous-répertoire apparait, celui-ci ainsi que ses sous-répertoires sont surveillés.

**Exemple: automd2h -w -r . va attendre les modifications des fichiers bar.md et baz/foo.bar.md. Les nouveaux fichiers .md dans . et dans bar seront aussi convertis. Les nouveaux répertoires dans . et dans bar seront aussi surveillés.**

Option -f
Par défaut, avec -w, les fichiers ne sont convertis que si une modification future est détectée.

Combiné avec -w, l'option -f force la conversion immédiate des fichiers trouvés puis surveille les modifications futures.

**Exemple: automd2h -w -f foo.txt convertit et crée le fichier foo.txt.html directement puis bloque et attend les modifications de foo.txt avant de reconvertir.**

Note: l'option -f sans -w est en fait le comportement par défaut.

Les options -w, -t et -f peuvent être combinées et forcent la conversion immédiate des fichiers trouvés s'ils sont plus récents que le fichier '.html' cible puis surveille les modifications futures. Cette combinaison d'options est intéressante car elle convertit immédiatement (-f) les fichiers qu'il faut mettre à jour (-t) puis attend les modifications avant de reconvertir (-w).

Combiné avec -r et -w, l'option -f convertit immédiatement les fichiers .md trouvés dans dans des nouveaux sous-répertoires de répertoires surveillés.

## Code de retour
Si un des fichiers n'a pas pu être correctement converti par pandoc (inexistant, problème de format, problème de lecture, problème d'écriture, etc.) automd2h laisse pandoc afficher les erreurs et continue le traitement. Néanmoins, automd2h retournera 1 lors de sa terminaison.

En cas de problème avec un fichier ou un répertoire particulier, un message d'erreur est affiché et l'outil continue le traitement. Néanmoins, automd2h retournera 1 lors de sa terminaison.

## Développement de l'outil
L'outil possède 5 options (-n, -t, -r, -w et -f) et chacune des combinaisons des ces options est possible et a du sens. Il est donc important de correctement factoriser et organiser votre code pour que chacune des options puisse fonctionner autant indépendamment qu'ensemble.

Il est conseillé de développer l'outil en suivant les étapes suivantes

## Fichier simple (et option -n)
Implémentez seulement le fait que les fichiers passés en argument sont des fichiers simples.

Pandoc doit être invoqué par votre programme. Vous utiliserez l'option -o de pandoc pour indiquer le fichier de sortie. La conversion par défaut de pandoc est suffisante dans le cadre du TP (pas besoin d'ajouter d'option de format). Important: vous devez utiliser fork, exec, etc., pour invoquer pandoc (pas de system ni de popen).

Si un fichier pose un problème pour pandoc, laissez pandoc gérer l'erreur et afficher des messages d'erreur.

Implémentez aussi l'option -n qui affiche le chemin du fichier source au lieu d'invoquer pandoc.

## Gestion du temps (option -t)
Implémentez l'option -t pour prendre en compte les dates des fichiers sources et cibles.

Vous utiliserez l'appel système stat pour déterminer les dates de modification des fichiers.

## Visite des répertoires
Implémentez la visite non récursive des répertoires (sans les options -r ni -w) pour trouver les fichiers .md a convertir si un fichier en argument est un répertoire.

Vous utiliserez les fonctions opendir, readdir, etc. pour trouver les fichiers .md à convertir.

## Notification simple (option -w)
Utilisez le mécanisme inotify du noyau Linux pour surveiller les fichiers à convertir.

RTFM: man inotify pour les détails de l'API

Attention, l'option -w ne convertit que les fichiers modifiés aperçus par inotify.

## Notification de répertoire
Adaptez votre gestion de la notification pour surveiller des répertoires en plus des fichiers.

Les modifications et ajouts des fichiers .md dans un répertoire surveillé entrainent une conversion.

## WTFN (option -f)
Implémenter l'option -f qui combiné avec -w couple la visite des répertoires (pour la conversion initiale) et la surveillance des répertoires (pour les modifications et ajouts de fichier .md).

Les options -w, -t, -f et -n ainsi que toutes leurs combinaisons doivent fonctionner à ce point-là.

## Visite récursive (option -r)
Implémentez la visite récursive des répertoires afin de convertir et surveiller les fichiers .md des les sous-répertoires et d'identifier les créations, déplacements et renommages de sous-répertoires.

## Réalisation
La réalisation de cet utilitaire se fera par groupes d'au plus deux personnes. Chaque membre de l'équipe devra maîtriser tous les aspects du programme.

Le programme devra être réalisé en C en utilisant principalement les appels système UNIX vus en classe. Le code source final (correctement commenté, factorisé et nettoyé) devra tenir dans un seul fichier C.

## Évaluation
Seront notamment pris en compte dans l'évaluation les critères suivants :

- Le bon fonctionnement de l'outil par rapport à la spécification donnée ;
- La qualité des solutions techniques proposées (indépendamment de leur implémentation) ;
- La qualité du code fourni (clarté, commentaires, indentation, factorisation, modularité, etc.) ;
- La bonne utilisation des appels système et le traitement systématique des cas d'erreurs ;
- L'exactitude et la robustesse de l'utilitaire face à des cas limites d'utilisation.
  
En particulier, les manipulations robustes des fichiers doivent prendre en compte l'existence de nombreuses possibilités d'erreurs et de situations problématiques, en particulier les situations de concurrence où un fichier n’existe plus ou a été modifié entre le moment où on le trouve et le moment où on le traite.

Un zéro vous sera automatiquement attribué si:

- Le programme ne compile pas
- Le programme ne fonctionne pas pour un fichier simple (par exemple automd2h foo.md
- Le code source du programme est inacceptable (indentation, factorisation, etc.)
- Le listing papier est illisible (lignes brisées, fonte non fixe, etc.)
- Note : un jeu de tests vous sera fourni.

## Jeu de tests
Un jeu de test est disponible sur la machine java.labunix.uqam.ca. Exécutez la commande ~privat_j/inf3173/t à partir d'un répertoire contenant votre binaire automd2h pour lancer la suite de tests.

## Document à rendre et dates
Date de remise électronique : dimanche 8 mars 23h55

Listing papier du programme en C à rendre en cours le lundi 9 mars (pas d'enveloppe, agrafes uniquement)

Remise en ligne du source en C via Moodle. Nom du fichier à soumettre : automd2h.c

Note importante : avant de soumettre votre travail, vérifiez bien que (i) votre programme compile et s'exécute correctement ; (ii) qu'un simple cat de votre programme affiche quelque chose de lisible, de compréhensible et de correctement indenté.
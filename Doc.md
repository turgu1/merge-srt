# Pour compiler l'application

```sh
$ ./build.sh
```

# Pout traiter un fichier

```sh
$ ./merge-srt J_C.srt J_C_data.csv output.srt
```

# Limitations

- Longueur des lignes maximum: 512 octets
- Nombre de lignes dans le fichier csv: 10,000
- Nombre de lignes dans le fichier srt: illimité

L'application est contenue dans un seul fichier source: `merge-srt.c`

Le nom des champs du fichier csv sont dans la structure de données nommée `cols_struct`. Le libellé des champs doit débuter exactement par les chaines identifiées dans le champs `cols[].csv_label`. L'application trouve automatiquement l'index des colonnes requises par la structure de données (l'index est placé dans le champ `cols_struct.col`). Il est donc possible d'ajouter/retrancher des colonnes au fichier csv sans nécessiter de changement au programme. Cependant, si le libellé d'une colonne requise est modifiée, l'entrée de données de `cols` doit être ajustée en conséquence.

Pour modifier (ajouter/retrancher) des colonnes à récupérer, il faut modifier la constante COLUMNS_COUNT ainsi que le contenu du vecteur `cols`. 
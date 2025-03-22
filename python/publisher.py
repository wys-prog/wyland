import os
import shutil
import wyan

def copy_files(src_dir, dest_dir):
    # Créer le répertoire de destination s'il n'existe pas
    if not os.path.exists(dest_dir):
        os.makedirs(dest_dir)

    # Parcourir tous les fichiers dans le répertoire source
    for item in os.listdir(src_dir):
        src_path = os.path.join(src_dir, item)
        dest_path = os.path.join(dest_dir, item)

        # Copier les fichiers uniquement (ignorer les sous-répertoires)
        if os.path.isfile(src_path):
            shutil.copy2(src_path, dest_path)
            print(f"Copied {src_path} to {dest_path}")

if __name__ == "__main__":
  src_directory = "./../builds/"
  dest_directory = wyan.REPO_PATH + '/res/'
  copy_files(src_directory, dest_directory)  
  wyan.main()
  

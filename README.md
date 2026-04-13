git init
git add .
git commit -m ""
git remote add origin link
git branch -M main
git pull origin main --allow-unrelated-histories ; only if push shows the error
git push -u origin main


git checkout -b branch_name
git branch
git checkout branch_name


@echo off
::git remote add github http://.....

::make sure goertek is laterst
git pull origin master

git fetch github master
git merge github master --no-commit

::git commit --amend --author 'faker <faker.yang@goertek.com>'
::git push origin HEAD:refs/for/master

pause
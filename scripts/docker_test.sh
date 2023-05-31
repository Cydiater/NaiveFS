docker build -t nfs .
docker run --rm nfs sh scripts/test.sh

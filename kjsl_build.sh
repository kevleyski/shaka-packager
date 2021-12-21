#!/bin/bash

# Kev's build script
HERE=$PWD

IMAGE_STATE=$(docker images -q kjsl_shaka_builder:latest 2> /dev/null)
RUN_STATE=$(docker ps -qf "ancestor=kjsl_shaka_builder")

create_docker_image () {
  if [[ "$IMAGE_STATE" == "" ]]; then
    docker build -t kjsl_shaka_builder .
  fi
}

run_docker_container () {
  if [[ "$RUN_STATE" == "" ]]; then
    echo "Spinning up new container"
    docker run --rm --name kjsl_shaka_builder --cap-add sys_ptrace -p127.0.0.1:2222:22 -d -v "$HERE:/host" kjsl_shaka_builder
    sleep 2

    # update running state image id to be the newly created Docker container
    RUN_STATE=$(docker ps -qf "ancestor=kjsl_shaka_builder")

    ssh-keygen -f "$HOME/.ssh/known_hosts" -R [localhost]:2222
  fi

  echo "Rejoining container $RUN_STATE"
  docker exec -i -t "$RUN_STATE" /bin/bash
}

create_docker_image
run_docker_container
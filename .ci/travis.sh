#!/bin/sh

# https://github.com/travis-ci/travis-rubies/blob/9f7962a881c55d32da7c76baefc58b89e3941d91/build.sh#L38-L44

travis_fold() {
  echo -e "travis_fold:start:$1\033[33;1m$2\033[0m"
}
travis_endfold() {
  echo -e "\ntravis_fold:end:$1\r"
}

travis_time_start() {
  travis_timer_id=$(printf %08x $(( RANDOM * RANDOM )))
  travis_start_time=$(travis_nanoseconds)
  echo -en "travis_time:start:$travis_timer_id\r${ANSI_CLEAR}"
}

travis_time_finish() {
  local result=$?
  travis_end_time=$(travis_nanoseconds)
  local duration=$(($travis_end_time-$travis_start_time))
  echo -en "\ntravis_time:end:$travis_timer_id:start=$travis_start_time,finish=$travis_end_time,duration=$duration\r${ANSI_CLEAR}"
  return $result
}

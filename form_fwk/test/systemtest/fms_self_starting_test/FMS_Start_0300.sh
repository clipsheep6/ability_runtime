# Copyright (c) 2021-2022 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
set -e
testfile="./formmgr/fms_self_starting_test_config.json"
sleep_seconds=10

rm -rf ./formmgr
mkdir ./formmgr

chown system ./formmgr
chgrp system ./formmgr

touch ${testfile}
echo \{\"AddForm\":true,\"DeleteForm\":false,\"Compare\":false\} > ${testfile};
chown system ${testfile}
chgrp system ${testfile}

sleep 2
echo "acquire forms"
./FmsSelfStartingTest

sleep 2
echo "kill foundation"
pgrep foundation | xargs kill -9

echo "sleep ${sleep_seconds} seconds"
sleep ${sleep_seconds}

sleep 2
before_kill=$(pgrep foundation)
echo ${before_kill} > "./formmgr/before_kill.txt"
echo "kill foundation"
pgrep foundation | xargs kill -9

echo "sleep ${sleep_seconds} seconds"
sleep ${sleep_seconds}

after_kill=$(pgrep foundation)
echo ${after_kill} > "./formmgr/after_kill.txt"
if [ "${before_kill}" == "${after_kill}" ]; then
    echo "process has not changed"
else
    echo "process has changed"
fi

rm -f ${testfile}
touch ${testfile}
echo \{\"AddForm\":false,\"DeleteForm\":false,\"Compare\":true\} > ${testfile};
chown system ${testfile}
chgrp system ${testfile}
sleep 12
echo "compare forms"
./FmsSelfStartingTest

rm -f ${testfile}
touch ${testfile}
echo \{\"AddForm\":false,\"DeleteForm\":true,\"Compare\":false\} > ${testfile};
chown system ${testfile}
chgrp system ${testfile}
sleep 2
echo "delete forms"
./FmsSelfStartingTest

rm -rf ./formmgr

return 0

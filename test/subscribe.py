import asyncio_redis
import redis
from time import sleep
import json

class DataBox:
    def __init__(self, ip="127.0.0.1", port=9379, passwd=""):#, passwd="hello", logger=None
        self.ip = ip
        self.port = port

    async def subscribe_md(self, channels, OnMarketData=None):
        # Create connection
        connection = await asyncio_redis.Connection.create(host=self.ip, port=self.port,
                                                           password=self.passwd)#,decode_responses=True
        # Create subscriber.
        subscriber = await connection.start_subscribe()
        # Subscribe to channel.
        newChannels = []
        for channel in channels:
            if '*' == channel:
                print('* is forbiddened to use when subscribe md')
                continue
            newChannels.append(channel)
        await subscriber.psubscribe(newChannels)
        # Inside a while loop, wait for incoming events.
        while True:
            try:
                reply = await subscriber.next_published()
                if OnMarketData:
                    try: 
                        md = yyjson.loads(reply.value)
                        OnMarketData(reply.channel, md)
                    except Exception as e:
                        print(traceback.format_exc(), reply.value)
            except Exception as e:
                print(traceback.format_exc())


def on_subscribe_data(channel, content):
    print(content)


if __name__ == '__main__':
    print('start test!')
    
    f = open('pub_info.json', 'w')
    re = redis.StrictRedis(host='35.74.79.161', port='9379')
    pub = re.pubsub()
    pub.subscribe('RM.StrategyAccount')
    while 1:
        content = pub.parse_response()
        if content:
            if type(content[2]) is not int:
                data = json.loads(content[2].decode('UTF-8'))
                account_list = data['data']
                account_array = account_list['list']
                print(account_array)

                for account in account_array:
                    if account['customer_id'] == 80071 or account['customer_id'] == 66666:
                        f.write(str(account))
                        f.write('\n\n')
                print('----------------------------------')
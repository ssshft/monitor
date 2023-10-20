import requests
import time
import hashlib
import hmac
import random
import base64
from hashlib import sha256
import json


def pb_rest_test():
    app_id = 'FzG2u1rt'
    app_secret = 'cbb9358aff75e1d999492cba99345ecf300894f3'
    current_time = int(time.time())
    method = 'POST'
    url = 'http://8.210.8.28/ideg-bp-se/api/alert/msg'

    content = {
        "notifyType": "1",
        "content": "content test",
        "receiverType": "0",
        "receiverName": "",
        "receiverIds": "oc_d3fcc1fa02395331deef609511b3808a",
        "notifyPlatform": "0",
        "title": "title test"
    }
    content = json.dumps(content, separators=(',', ':'))

    nonce = random.randint(100000000, 999999999)

    data = {
        "data": content,
        "signature": "",
        "appId": app_id,
        "nonce": nonce,
        "timestamp": current_time
    }

    para = []
    keys = sorted(data.keys())
    for k in keys:
        if k == 'signature':
            continue
        s = f'{k}={data[k]}'
        para.append(s)
    body = '&'.join(para)
    print(body)

    signature = base64.b64encode(hmac.new(app_secret.encode('utf-8'), body.encode('utf-8'), digestmod=sha256).digest())
    headers = {'Accept': 'application/json', 'Content-Type': 'application/json'}

    signature = signature.decode('utf-8')

    data['signature'] = signature
    data = json.dumps(data)
    print(data)


    response_data = requests.request(method, url, data=data, headers=headers)
    print(response_data.text)


def coinbase_test():
    api_key = 'ZdlFsitVWo4=xc3wftV5SjupDnikfa3FKV8eGaD4ze8EliVvISnjBGs='
    secret_key = '/NqFL5Fk4qM=eFix0K/ZBumSkkhBP4yQPhAPBWnzXxNr2GxbS0+VAT6INq6FpAjys/K4nVjwqgr77aBDDrC3NDbC7fMb/OrUPw=='
    passphrase = '5Rb5KFBCGLQ=T/7FtI/zpYsNYUWJjsJQCA=='

    timestamp = str(int(time.time())) 
    timestamp = '1660630533'

    url = "https://api.prime.coinbase.com/v1/portfolios"
    request_path = '/v1/portfolios'
    body = {}

    message = timestamp + 'GET' + request_path
    signature = hmac.new(secret_key.encode('utf-8'), message.encode('utf-8'), digestmod=hashlib.sha256).digest() 
    signature_b64 = base64.b64encode(signature).decode()

    print(signature_b64)
    

    # headers = {
    #     "Accept": "application/json",
    #     "X-CB-ACCESS-KEY": api_key,
    #     "X-CB-ACCESS-PASSPHRASE": passphrase,
    #     "X-CB-ACCESS-SIGNATURE": signature_b64,
    #     "X-CB-ACCESS-TIMESTAMP": timestamp
    # }

    # response = requests.get(url, headers=headers)

    # print(response.text)

    # balances
    # url = "https://api.prime.coinbase.com/v1/portfolios/portfolio_id/balances"

    # headers = {
    #     "Accept": "application/json",
    #     "X-CB-ACCESS-KEY": "$ACCESS_KEY",
    #     "X-CB-ACCESS-PASSPHRASE": "$PASSPHRASE",
    #     "X-CB-ACCESS-SIGNATURE": "$SIGNATURE",
    #     "X-CB-ACCESS-TIMESTAMP": "$TIMESTAMP"
    # }

    # response = requests.get(url, headers=headers)

    # print(response.text)

def market_price():

    url = "https://api.exchange.coinbase.com/oracle"

    api_key = '0GQwqlT4247VcPk9SaKYrqgqmO2yxhdGIacnrVXvEDKXgPsSiIFh2AsWEw9knegg'
    secret_key = 'ramZAj5HnzljlwO9v4Y30WLW5Fgo4I0ZiAbeveeuhd9BasTHLlJfQVnrkhbKcdkM'
    passphrase = '123456'

    timestamp = str(int(time.time())) 

    message = timestamp + 'GET' + url + '' 
    signature = hmac.new(secret_key.encode('utf-8'), message.encode('utf-8'), digestmod=hashlib.sha256).digest() 
    signature_b64 = base64.b64encode(signature).decode()

    headers = {
        "Accept": "application/json",
        "cb-access-key": api_key,
        "cb-access-passphrase": passphrase,
        "cb-access-sign": signature_b64,
        "cb-access-timestamp": timestamp
    }

    response = requests.get(url, headers=headers)

    print(response.text)


if __name__ == '__main__':
    print('start test!')
    coinbase_test()


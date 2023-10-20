from api.larkcustom import LarkCustomBot
import asyncio

webhook_url = 'https://open.larksuite.com/open-apis/bot/v2/hook/ae37233c-b5b1-4b9a-a9f4-820436367989'
webhook_secret = 'PaQY1h1MURrO24YZaYNlPb'

if __name__ == '__main__':
    lark = LarkCustomBot(webhook_url, webhook_secret)
    asyncio.run(lark.send_text("this is a test!", True))
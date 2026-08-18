from fastapi import FastAPI, Depends, HTTPException, status, Request
from fastapi.security import HTTPBasic, HTTPBasicCredentials
from fastapi.responses import FileResponse
from pydantic import BaseModel
import uvicorn

import json
from typing import Annotated

import time

app = FastAPI()

security = HTTPBasic()

class Send(BaseModel):
    sender: str
    receiver: str
    message: str

def check_user(credentials: HTTPBasicCredentials):
    with open("users.json", "r") as f:
        users = json.load(f)

    user = users.get(credentials.username)

    if not user or user["password"] != credentials.password:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid credentials",
            headers={"WWW-Authenticate": "Basic"},
        )

    return credentials.username
@app.post("/api/checkuser")
async def checkuser(credentials: Annotated[HTTPBasicCredentials, Depends(security)]):
    f = check_user(credentials)
    return {"status": "ok"}

@app.post("/api/send")
async def send(
    send: Send,
    credentials: Annotated[HTTPBasicCredentials, Depends(security)]
):
    sender = check_user(credentials)

    with open("messages.json", "r") as f:
        messages = json.load(f)

    messages.append({
        "sender": sender,
        "receiver": send.receiver,
        "message": send.message,
        "timestamp": time.time()
    })

    with open("messages.json", "w") as f:
        json.dump(messages, f, indent=2)

    return {"status": "sent"}

@app.get("/api/inbox")
async def inbox(credentials: Annotated[HTTPBasicCredentials, Depends(security)]):
    user = check_user(credentials)

    with open("messages.json", "r") as f:
        messages = json.load(f)

    inbox = [
        m for m in messages
        if m["receiver"] == user
    ]

    return {"inbox": inbox}
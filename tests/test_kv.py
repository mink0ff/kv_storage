import pytest
import time

async def test_set_and_get(service_client):
    response = await service_client.post(
        "/kv/set",
        json={"key": "foo", "value": "bar"},
    )
    assert response.status == 200
    data = response.json()
    assert data["result"] == "success"
    assert data["key"] == "foo"
    assert data["value"] == "bar"

    response = await service_client.get("/kv/get", params={"key": "foo"})
    assert response.status == 200
    data = response.json()
    assert data["found"] is True
    assert data["value"] == "bar"


async def test_del_existing(service_client):
    response = await service_client.delete("/kv/del", params={"key": "foo"})
    assert response.status == 200
    data = response.json()
    assert data["deleted"] is True
    assert data["key"] == "foo"

    response = await service_client.get("/kv/get", params={"key": "foo"})
    assert response.status == 404
    data = response.json()
    assert data["found"] is False


async def test_del_nonexistent(service_client):
    response = await service_client.delete("/kv/del", params={"key": "not_found"})
    assert response.status == 404
    data = response.json()
    assert data["deleted"] is False
    assert data["key"] == "not_found"
    assert data["error"] == "Key not found"

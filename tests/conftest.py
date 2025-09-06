import pathlib
import pytest

pytest_plugins = ["pytest_userver.plugins.core"]

@pytest.fixture(scope="session")
def service_env():
    """Очистка/создание тестовых директорий перед запуском сервиса"""
    base = pathlib.Path(__file__).parent.resolve()  
    test_data = base / "test_data"

    snapshots = test_data / "snapshots"
    aof = test_data / "aof/appendonly.aof"

    if snapshots.exists():
        for f in snapshots.glob("*"):
            f.unlink()
    else:
        snapshots.mkdir(parents=True)

    if aof.exists():
        aof.unlink()
    else:
        aof.parent.mkdir(parents=True, exist_ok=True)

    return {}
    

@pytest.fixture(scope="session")
def service_config_path():
    return pathlib.Path(__file__).parent / "../configs/static_config_test.yaml"

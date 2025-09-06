import pathlib
import pytest

pytest_plugins = ["pytest_userver.plugins.core"]

@pytest.fixture(scope="session")
def service_env():
    """Очистка/создание тестовых директорий перед запуском сервиса"""
    snapshots = pathlib.Path("./test_data/snapshots")
    aof = pathlib.Path("./test_data/appendonly.aof")

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
    return pathlib.Path(__file__).parent / "static_config_test.yaml"

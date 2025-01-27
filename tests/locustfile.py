# Usage: locust -f locustfile.py --headless --users 10 --spawn-rate 1 -H http://localhost:8000
from locust import HttpUser, task, between


class QuickstartUser(HttpUser):
    wait_time = between(1, 5)

    @task(1)
    def home(self):
        self.client.get("/", name="/")

    @task(1)
    def get_docs(self):
        self.client.get("/docs", name="/docs")

    @task(3)
    def get_xsl_list_only(self):
        self.client.get("/saved-xsl-list-only", name="/xsl_list_only")

    @task(5)
    def not_found(self):
        self.client.get("/not-exist-endpoint", name="/not-exist-endpointAAA")

    @task(3)
    def not_found2(self):
        self.client.get("/not-exist-endpoint2", name="/not-exist-endpointBBB")




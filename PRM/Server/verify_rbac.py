import requests
import json

BASE_URL = "http://localhost:8080" # Let's assume the server is on 8080. Wait, E2E used 8099, but Application.cpp default is 8080.

def print_result(desc, res, expected_status):
    success = res.status_code == expected_status
    print(f"[{'PASS' if success else 'FAIL'}] {desc} | Expected {expected_status}, Got {res.status_code}")
    if not success:
        print(f"Response: {res.text}")
    return success

# 1. Public routes still work.
res = requests.get(f"{BASE_URL}/health")
print_result("Public route /health works", res, 200)

# 2. Missing token returns 401.
res = requests.get(f"{BASE_URL}/users")
print_result("Missing token returns 401", res, 401)

# 3. Invalid token returns 401.
headers = {"Authorization": "Bearer invalid_token_xyz"}
res = requests.get(f"{BASE_URL}/users", headers=headers)
print_result("Invalid token returns 401", res, 401)

# Generate valid tokens
def register_and_login(username, role):
    requests.post(f"{BASE_URL}/auth/register", json={
        "username": username,
        "password": "Password123!",
        "email": f"{username}@example.com",
        "role": role,
        "full_name": "Test User"
    })
    res = requests.post(f"{BASE_URL}/auth/login", json={
        "username": username,
        "password": "Password123!"
    })
    if res.status_code == 200:
        return res.json()["token"]
    else:
        # User already exists, try logging in
        pass
    return None

emp_token = register_and_login("test_emp123", "EMPLOYEE")
mgr_token = register_and_login("test_mgr123", "MANAGER")
adm_token = register_and_login("test_adm123", "ADMIN")

emp_headers = {"Authorization": f"Bearer {emp_token}"}
mgr_headers = {"Authorization": f"Bearer {mgr_token}"}
adm_headers = {"Authorization": f"Bearer {adm_token}"}

# 4. EMPLOYEE cannot access admin routes.
res = requests.post(f"{BASE_URL}/users", headers=emp_headers, json={"username": "foo"})
print_result("EMPLOYEE cannot access POST /users (Admin)", res, 403)

# 5. EMPLOYEE cannot access manager routes.
res = requests.post(f"{BASE_URL}/allocations", headers=emp_headers, json={"project_id": 1})
print_result("EMPLOYEE cannot access POST /allocations (Manager)", res, 403)

# 6. MANAGER cannot access admin routes.
res = requests.post(f"{BASE_URL}/users", headers=mgr_headers, json={"username": "foo"})
print_result("MANAGER cannot access POST /users (Admin)", res, 403)

# 7. ADMIN can access all admin routes.
# Because the payload is missing other required fields, we expect 400 Validation Error, NOT 403 Forbidden!
res = requests.post(f"{BASE_URL}/users", headers=adm_headers, json={"username": "foo"})
print_result("ADMIN can access POST /users (gets 400 instead of 403)", res, 400)

# 8. Valid roles can access allowed endpoints.
res = requests.get(f"{BASE_URL}/projects", headers=emp_headers)
print_result("EMPLOYEE can access GET /projects (gets 200)", res, 200)

res = requests.get(f"{BASE_URL}/projects", headers=mgr_headers)
print_result("MANAGER can access GET /projects (gets 200)", res, 200)

res = requests.get(f"{BASE_URL}/projects", headers=adm_headers)
print_result("ADMIN can access GET /projects (gets 200)", res, 200)

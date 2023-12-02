<?php
if($_SERVER["REQUEST_METHOD"] =="POST"){
    $username = $_POST['username'];
    $phone = $_POST['phone'];
    $email = $_POST['email'];
    $password = $_POST['password'];
    $confirmpassword = $_POST['confirm-password'];
    echo $username . $phone;
}
?>
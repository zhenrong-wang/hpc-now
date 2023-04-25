terraform {
  required_providers {
    aws = {
      source = "hashicorp/aws"
      version = "~> 4.0"
    }
  }
}
provider "aws" {
  access_key = "BLANK_ACCESS_KEY_ID"
  secret_key = "BLANK_SECRET_KEY"
  region = "cn-northwest-1"
}

data "aws_vpc" "test" {
  default = "true"
}